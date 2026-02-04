ESP32 BTHome Receiver
=====================

**BTHome** is an energy efficient but flexible BLE (Bluetooth Low Energy) format for devices to
broadcast their sensor data and  button presses. Devices can run over a year on a single battery.
It allows data encryption and is supported by popular home automation platforms,
like `Home Assistant <https://www.home-assistant.io>`__, out of the box.

For more information, including the full protocol specification and all supported sensor types, visit
`BTHome.io <https://bthome.io>`__

This component implements local reception and decoding without the need of a central hub, instead parsing
the packets directly on the ESP microcontroller.


Note: as for now this component is limited to V1 and V2 unencryted formats.
Encryption support might be implemented later on.

.. code-block:: yaml

    # Example configuration entry
    external_components:
      - source: github://juleskers/esphome_component_bthome

    # The basic ESP Bluetooth low-energy stack, needed to receive BLE packets at all.
    esp32_ble_tracker:

    # The BThome-specific logic, provided by this repo.
    # Parses messages received by the basic tracker stack.
    bthome_ble_receiver:
      dump: unmatched
      devices:
        - mac_address: 11:22:33:44:55:55
          name_prefix: BTHome TestDevice
          expected_remote_passkey: 0x1234
          dump: all

    sensor:
      - platform: bthome_ble_receiver
        mac_address: 11:22:33:44:55:55
        sensors:
          - measurement_type: temperature
            name: Temperature
          - measurement_type: humidity
            name: Humidity
          - measurement_type: 0x02
            name: Temperature_Second
            accuracy_decimals: 2
            unit_of_measurement: °C

      - platform: bthome_ble_receiver
        mac_address: 22:33:44:55:55:66
        sensors:
          - measurement_type: temperature
            name: BTHome Garage Temperature

    binary_sensor:
      - platform: bthome_ble_receiver
        mac_address: 11:22:33:44:55:55
        name_prefix: BTHome Garage
        sensors:
          - measurement_type: opening
            name: Opening

.. _bthome-component:

Component/Hub
-------------

The ``bthome_ble_receiver`` component creates a global hub so that you can track bluetooth low
energy devices using your ESP32 node over the BTHome protocol using both v1 and v2 protocols.

The component depends on the ``esp32_ble_tracker`` component which needs to be added to the
configuration.

The bthome receiver component is an internal model that acts as a central reception
and dispatcher hub to which bthome virtual devices and sensors are connected to.
Additionally, the hub is responsible for handling of BTHome events, which have no other
clean mapping in the ESPHome world.

.. _config-bthome:

Configuration variables:
************************

- **dump** (*Optional*): Decode and dump incoming remote readings codes in the logs
  (at log.level=DEBUG) for any device.

  - **all**: Decode and dump all readings.
  - **unmatched**: Decode and dump readings that are not mapped in configuration.
  - **none**: (default) Decode but do not dump any readings.

- **id** (*Optional*): Manually specify the ID for this Hub.

- **devices** (*Optional*): List of remote devices connected to this hub. Any devices not specified here, but only in sensor sections will use default settings.

  - **mac_address** (**Required**, mac-address): The address of the sensor.

  - **name_prefix** (*Optional*): Device name to append before any sensor name as a prefix.

  - **dump** (*Optional*): Decode and dump incoming remote readings codes in the logs
    (at log.level=DEBUG) for this device.

  - **encryption_key** (*Optional*): Encryption key for the device.

.. _bthome-events:

Events: Packets, Buttons and Dimmers
************************************

The hub may specify event handlers. As of this writing,
the BTHome specification only supports button-click and dimmer-rotation events,
for momentary push-buttons and twistable knobs respectively.
Rather than a value, as for sensors, events have an `event type` (e.g. "single click", "long press", "rotate left").
Dimmer events have an additional byte specifying how many steps the rotation was (that is, "rotate left", "5 steps").

In addition to the BTHome event types, this codebase also supports an `on_packet` handler, as hook for fully
custom packet handling.

Configuration variables:
^^^^^^^^^^^^^^^^^^^^^^^^

All Event types may be specified either at the topmost `bthome_ble_receiver` level
(responding to *any* sender of BTHome packets), or under a specific `devices` entry (limited to events
sent by that mac address).

- **on_event**: Specifies a generic event handler, triggering for any BTHome event.
  The handling trigger receives the sending MAC-address and the full data of the pre-parsed event,
  as represented by the `bthome_base::bthome_measurement_event_record_t` C-struct.

- **on_<event_type>**: These specific handler trigger only for one specific type of event,
  named by the type following the `on_` prefix. For example: **on_button_click** will respond only to a
  single-clicked button.

  These typed handlers have access to the same MAC+event data as the generic `on_event` handler.

  Supported `on_`-button events are: `on_button_click`, `on_button_double_click`,
  `on_button_triple_click`, `on_button_long_click`, `on_button_long_double_click`,
  `on_button_long_triple_click`, `on_button_hold_click` and finally `on_button_none`.
  Furthermore, dimmer events support `on_dimmer_rotate_right`, `on_dimmer_rotate_left` and `on_dimmer_none`.

- **on_packet**: Triggers whenever any valid BTHome packet is received, regardless of its contents.
  It may even be a packet that, from a BTHome perspective is "empty". That is, containing zero sensors or events,
  only headers.
  This is provided as a hook-point for fully-custom packet parsing, in case the tools provided by this repository
  do not suffice.

  Packet event handlers receive the sending MAC-address and a vector of pre-parsed `bthome_measurement_record_t`,
  which can contain a mix of value-records (`bthome_measurement_value_record_t`) and event-records
  (`bthome_measurement_event_record_t`).

.. _bthome-sensor:

Sensor and Binary Sensor as *virtual device*
--------------------------------------------

The bthome sensor allows you use a sensor to display received measurement from a remote
BTHome device.
First, you need to define a :ref:`bthome hub component <bthome-component>`.

The bthome sensor component (or "device") is an internal model that acts as a central reception
and dispatcher hub for a specific remote device identified by a ``mac_address`` to which bthome
sensors are connected to.

To initialize a sensor, first supply ``mac_address`` to identify the remote BTHome device.

.. code-block:: yaml

    # Example configuration entry
    bthome_ble_receiver:

    # Individual sensors
    sensor:
      - platform: bthome_ble_receiver
        mac_address: 11:22:33:44:55:55
        sensors:
          - measurement_type: temperature
            name: Temperature
          - measurement_type: humidity
            name: Humidity
          - measurement_type: 0x02
            name: Temperature_Second
            accuracy_decimals: 2
            unit_of_measurement: °C


.. _config-bthome-sensor:

Configuration variables:
************************

- **mac_address** (**Required**, mac-address): The address of the sensor.

- **sensors** (*Required*): List of remote sensor connected to this virtual device.

  - **name** (*Optional*): The name for the sensor. At least one of **id** and **name** must be specified.

  - **measurement_type** (*Required*, int **or** string): Measurement type as defined in
    `BTHome format specification <https://bthome.io/format>`__ either as a string or a numeric value.
    If selected by name (string) the accuracy and unit of measurement are automatically defaulted to the
    correct values.

    Measurement type `further details <bthome_common_format.rst>`__ to be taken into account.

See Also
--------

- `BTHome <https://bthome.io/>`__ by Ernst Klamer, Victor, Paulus Schoutsen.
