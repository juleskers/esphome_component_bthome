/*
 BTHome protocol virtual sensors for ESPHome

 Author: Attila Farago
 */

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/bthome_base/bthome_parser.h"
#include "mbedtls/ccm.h"

#include "bthome_ble_receiver_hub.h"

#ifdef USE_ESP32

namespace esphome
{
  namespace bthome_ble_receiver
  {
    static const char *const TAG = "bthome_ble_receiver";

    bool BTHomeBLEReceiverHub::parse_device(const esp32_ble_tracker::ESPBTDevice &device)
    {
      bool success = false;
      for (auto &service_data : device.get_service_datas())
      {
        bthome_base::BTProtoVersion_e proto = parse_header_(service_data);
        if (proto != bthome_base::BTProtoVersion_None)
        {
          if (parse_message_payload_(service_data, device, proto))
            success = true;
        }
      }

      return false;
    }

    bthome_base::BTProtoVersion_e BTHomeBLEReceiverHub::parse_header_(const esp32_ble_tracker::ServiceData &service_data)
    {
      // 0000181c-0000-1000-8000-00805f9b34fb, 0000181e-0000-1000-8000-00805f9b34fb
      // esp32_ble_tracker::ESPBTUUID::from_uint16(0x181C)
      if (service_data.uuid.contains(0x1C, 0x18))
        return bthome_base::BTProtoVersion_BTHomeV1; // unencrypted: 0000181c, encrypted: 0000181e
      else if (service_data.uuid.contains(0xD2, 0xFC))
        return bthome_base::BTProtoVersion_BTHomeV2; // 0000fcd2
      else
        return bthome_base::BTProtoVersion_None;

      // auto raw = service_data.data;
      // static uint8_t last_frame_count = 0;
      // if (last_frame_count == raw[13]) {
      //   ESP_LOGVV(TAG, "parse_header(): duplicate data packet received (%hhu).", last_frame_count);
      //   return {};
      // }
      // last_frame_count = raw[13];
    }

    bool BTHomeBLEReceiverHub::parse_message_payload_(const esp32_ble_tracker::ServiceData &service_data, const esp32_ble_tracker::ESPBTDevice &device, bthome_base::BTProtoVersion_e proto)
    {
      // Check and match the device
      const mac_address_t address = device.address_uint64();

      // Get BTHome device configuration (reused for encryption key lookup)
      auto *btdevice = get_device_by_address(address);

      // Parse the payload data
      std::vector<uint8_t> message = service_data.data;
      const uint8_t *payload_data = message.data();
      auto payload_length = message.size();

      if (proto == bthome_base::BTProtoVersion_BTHomeV1)
      {
        // NOOP
      }
      else if (proto == bthome_base::BTProtoVersion_BTHomeV2)
      {
        uint8_t adv_info = payload_data[0];
        bool encryption = bool(adv_info & (1 << 0));       // bit 0
        bool mac_included = bool(adv_info & (1 << 1)); // bit 1
        // bool sleepy_device = bool(adv_info & (1 << 2));    // bit 2
        uint8_t sw_version = (adv_info >> 5) & 7; // 3 bits (5-7);

        if (proto != sw_version)
        {
          ESP_LOGD(TAG, "BTHome V2 device is using an unsupported sw_version %d.", sw_version);
          return false;
        }

        if (encryption)
        {
          // Check if encryption key is configured for this device
          const uint8_t *encryption_key = nullptr;

          if (btdevice && btdevice->has_encryption_key())
          {
            encryption_key = btdevice->get_encryption_key();
          }
          else
          {
            ESP_LOGW(TAG, "Got encrypted msg but KEY NOT SET");
            return false;
          }

          if (!decrypt_message_payload_(message, encryption_key, address))
          {
            ESP_LOGD(TAG, "Encrypted msg failed decryption");
            return false;
          }

          payload_data = message.data();
          payload_length = message.size();
        }

        if (mac_included)
        {
          // bthome_receiver_mac_reversed = data[1:7]
          // mac_readable = to_mac(bthome_receiver_mac_reversed[::-1])
          payload_data += 7;
          payload_length -= 7;
        }
        else
        {
          // mac_readable = service_info.address;
          payload_data += 1;
          payload_length -= 1;
        }

      }

      // parse the payload and report measurements in the callback
      parse_message_bthome_(address, payload_data, payload_length, proto);

      return true; // TODO change parse_message_bthome_ to return bool
    }

    bool BTHomeBLEReceiverHub::decrypt_message_payload_(std::vector<uint8_t> &raw, const uint8_t *bindkey, const uint64_t &address) {
      ESP_LOGV(TAG, "attempting decryption");
      uint8_t mac_address[6] = {0};
      mac_address[0] = (uint8_t) (address >> 40);
      mac_address[1] = (uint8_t) (address >> 32);
      mac_address[2] = (uint8_t) (address >> 24);
      mac_address[3] = (uint8_t) (address >> 16);
      mac_address[4] = (uint8_t) (address >> 8);
      mac_address[5] = (uint8_t) (address >> 0);

      BTHomeAESVector vector{.key = {0},
                             .plaintext = {0},
                             .ciphertext = {0},
                             .authdata = {0},
                             .iv = {0},
                             .tag = {0},
                             .keysize = 16,
                             .authsize = 0,
                             .datasize = 0,
                             .tagsize = 4,
                             .ivsize = 13};

      vector.datasize = raw.size() - 9;
      int cipher_pos = 1;

      const uint8_t *v = raw.data();

      memcpy(vector.key, bindkey, vector.keysize);
      memcpy(vector.ciphertext, v + cipher_pos, vector.datasize);
      memcpy(vector.tag, v + raw.size() - vector.tagsize, vector.tagsize);
      memcpy(vector.iv, mac_address, 6);             // MAC address
      vector.iv[6] = 0xd2;
      vector.iv[7] = 0xfc;
      memcpy(vector.iv + 8, v, 1);                   // device data byte
      memcpy(vector.iv + 9, v + raw.size() - 8, 4);  // counter

      mbedtls_ccm_context ctx;
      mbedtls_ccm_init(&ctx);

      int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, vector.key, vector.keysize * 8);
      if (ret) {
        ESP_LOGV(TAG, "decrypt: mbedtls_ccm_setkey() failed.");
        mbedtls_ccm_free(&ctx);
        return false;
      }

      ret = mbedtls_ccm_auth_decrypt(&ctx, vector.datasize, vector.iv, vector.ivsize, vector.authdata, vector.authsize,
                                     vector.ciphertext, vector.plaintext, vector.tag, vector.tagsize);
      if (ret) {
        ESP_LOGV(TAG, "decrypt: failed, error : %d\n"
                       "   MAC: %s\n"
                       "Packet: %s\n"
                       "   Key: %s\n"
                       "    Iv: %s\n"
                       "Cipher: %s\n"
                       "   Tag: %s",
                       ret,
                       format_hex_pretty(mac_address, 6).c_str(),
                       format_hex_pretty(raw.data(), raw.size()).c_str(),
                       format_hex_pretty(vector.key, vector.keysize).c_str(),
                       format_hex_pretty(vector.iv, vector.ivsize).c_str(),
                       format_hex_pretty(vector.ciphertext, vector.datasize).c_str(),
                       format_hex_pretty(vector.tag, vector.tagsize).c_str()
                       );
        mbedtls_ccm_free(&ctx);
        return false;
      }

      // replace encrypted payload with plaintext
      uint8_t *p = vector.plaintext;
      for (std::vector<uint8_t>::iterator it = raw.begin() + cipher_pos;
           it != raw.begin() + cipher_pos + vector.datasize;
           ++it)
      {
        *it = *(p++);
      }

      // clear encrypted flag
      raw[0] &= ~0x08;

      // remove tag from the end of the payload
      raw.resize(raw.size() - 8);

      ESP_LOGV(TAG, "decrypt success");
      // plaintext at VV, because will also be logged by normal message handler.
      ESP_LOGVV(TAG, "  Plaintext : %s", format_hex_pretty(raw.data(), raw.size()).c_str());

      mbedtls_ccm_free(&ctx);
      return true;
    }

  }
}

#endif
