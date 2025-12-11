/*
 BTHome V1, V2 protocol packages parser

 Based on bthome_ble source parser by Ernst Klamer
 https://pypi.org/project/bthome-ble/

 Author: Attila Farago
 */

#include <vector>
#include <map>
#include <string>
#include <cstring>
#ifndef USE_ESP32
#include <pgmspace.h>
#endif

#include "bthome_parser.h"
#include "bthome_common_generated.h"

namespace bthome_base
{
  inline uint16_t combine_bytes_little_endian_u16(const uint8_t *data) { return ((data[1] & 0xFF) << 8) | (data[0] & 0xFF); }
  inline uint32_t combine_bytes_little_endian_u24(const uint8_t *data) { return ((data[2] & 0xFF) << 16) | ((data[1] & 0xFF) << 8) | (data[0] & 0xFF); }
  inline uint32_t combine_bytes_little_endian_u32(const uint8_t *data) { return ((data[3] & 0xFF) << 24) | ((data[2] & 0xFF) << 16) | ((data[1] & 0xFF) << 8) | (data[0] & 0xFF); }

  float parse_integer(const uint8_t *data, HaBleTypes_e obj_data_format, uint8_t data_length)
  {
    float value = {};
    bool is_signed = obj_data_format == HaBleType_sint;
    switch (data_length)
    {
    case 1:
    {
      uint16_t value1 = *data;
      return !is_signed ? value1 : signextend<int8_t, 8>(value1);
    }
    case 2:
    {
      uint16_t value1 = combine_bytes_little_endian_u16(data);
      return !is_signed ? value1 : signextend<int16_t, 16>(value1);
    }
    case 3:
    {
      uint32_t value1 = combine_bytes_little_endian_u24(data);
      return !is_signed ? value1 : signextend<int32_t, 32>(value1);
    }
    case 4:
    {
      uint32_t value1 = combine_bytes_little_endian_u32(data);
      return !is_signed ? value1 : signextend<int32_t, 32>(value1);
    }
    default:
      return 0;
    }
  }

  // parse BTHome v1 protocol data - https://bthome.io/v1/ - UUID16 = 0x181C
  // parse BTHome v2 protocol data - https://bthome.io/ - UUID16 = 0xfcd2
  bool parse_payload_bthome(const uint8_t *payload_data, uint32_t payload_length, BTProtoVersion_e proto,
                            measurement_cb_fn_t measurement_cb, log_cb_fn_t log_cb)
  {
    uint8_t next_obj_start = 0;         // pointer inside data array
    uint8_t prev_obj_meas_type = 255;   // Keep track of measurement types, to verify that ascending order is maintained; as per spec
    uint8_t obj_meas_type;              // Measurement Type; what is the sensor value representing?
    uint8_t obj_meas_type_offset = 0;   // for multiple occurrences of the same measurement type (e.g. multiple buttons): which one is this?
    uint8_t obj_control_byte;           // deprecated: used in deprecated BTHomeV1 protocol;
    uint8_t obj_data_length;            // How many bytes to consume for this measurement type?
    HaBleTypes_e obj_data_format;       // How to interpret the data-bytes? (uint? sint? 8, 16, 32 bits?)
    uint8_t obj_data_start;
    float obj_data_factor;              // Data multiplier / scaling factor associated with this measurement type

    if (log_cb)
    {
      // debug log full binary payload
      char buffer [4];
      char msg [70];
      int n;
      strcpy(msg, "rec BT payload is: ");
      for (int i = 0; i < payload_length; ++i)
       {
           n=sprintf (buffer, "%.2x ", payload_data[i]);
           strlcat(msg, buffer, sizeof(msg));
       }
       log_cb(msg);
    }


    // Iterate payload buffer until all fields are processed.
    while (payload_length >= next_obj_start + 1)
    {
      auto obj_start = next_obj_start;

      // Derive this item´s size from protocol version
      if (proto == BTProtoVersion_BTHomeV1)
      {
        // BTHome V1
        obj_meas_type = payload_data[obj_start + 1];
        // obj_control_byte = payload_data[obj_start];
        // obj_data_length = (obj_control_byte >> 0) & 31; // 5 bits (0-4)
        // obj_data_format = (obj_control_byte >> 5) & 7;  // 3 bits (5-7)
        obj_data_start = obj_start + 2;
      }
      else if (proto == BTProtoVersion_BTHomeV2)
      {
        // BTHome V2
        obj_meas_type = payload_data[obj_start];
        if (prev_obj_meas_type == obj_meas_type)
        {
          // repeated instance of same measurement type; increment index/offset
          obj_meas_type_offset += 1;
        } else {
          // non-repeat; restart offset for next measurement type.
          obj_meas_type_offset = 0;
        }

        if (prev_obj_meas_type > obj_meas_type)
        {
          if (log_cb)
            log_cb("BTHome device is not sending object ids in required ascending order (from low to high object id).");
        }

        prev_obj_meas_type = obj_meas_type;
        obj_data_start = obj_start + 1;
      }
      else
      {
        if (log_cb)
        {
          std::string message = "BTHome unsupported protocol version - ";
          message.append(std::to_string(proto));
          log_cb(message.c_str());
        }
        return false;
      }

      // Given protocol version: parse data fields

      // sanity check: do we have parsing info for this measurement type?
      if (obj_meas_type >= sizeof(MEAS_TYPES_FLAGS) / sizeof(uint8_t))
      {
        if (log_cb)
        {
          std::string message = "Object ID from the future found in payload, please regenerate constants for parsing code - ";
          message.append(std::to_string(obj_meas_type));
          log_cb(message.c_str());
        }
        break; // stop parsing, all subsequent IDs will also be higher than what we know how to handle.
      }

      // parsing instructions for this measurement-type
      BTHomeDataFormat dataformat = getDataFormat(obj_meas_type);
      obj_data_length = dataformat.len_in_bytes;
      obj_data_format = dataformat.data_format;
      obj_data_factor = dataformat.factor_multiple;
      next_obj_start = obj_data_start + obj_data_length;

      if (obj_data_length == 0)
      {
        if (log_cb)
          log_cb("Invalid payload data length found with length 0.");
        continue;
      }
      if (payload_length < next_obj_start)
      {
        // buffer overflow: reading this data field would take us beyond payload data.
        if (log_cb)
          log_cb("Invalid payload data length: ran into payload end while expecting more data.");
        break; // stop parsing, but don't consider earlier fields failed.
      }

      const uint8_t obj_value_data_length = obj_data_length;

      const uint8_t *data = &payload_data[obj_data_start];
      float value = 0;
      if (obj_data_format == HaBleType_uint || obj_data_format == HaBleType_sint)
      {
        value = (float)parse_integer(data, (HaBleTypes_e)obj_data_format, obj_value_data_length) / obj_data_factor;
      }
      else
      {
        if (log_cb)
        {
          std::string message = "Unknown payload data type - ";
          message.append(std::to_string(obj_data_format));
          log_cb(message.c_str());
        }
        continue;
      }

      // report measurement
      if (measurement_cb)
        measurement_cb(obj_meas_type, obj_meas_type_offset, value);
    }

    return true;
  }

}
