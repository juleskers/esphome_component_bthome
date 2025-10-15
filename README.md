# Maintenance Note

The original [afarago upstream](https://github.com/afarago/esphome_component_bthome) notes that the project has been archived, and the author no longer has interest in maintaining it.  
Since then, several forks have made various minor tweaks to the codebase.
This repo represents a review of all those (public) forks, and attempts to provide a new unified starting point.

I've reviewed and merged the entire public fork network of afarago's work: 7 forks with changes, not counting my own.
- [afarago/master](https://github.com/afarago/esphome_component_bthome) @ [2da6aff87b](https://github.com/afarago/esphome_component_bthome/tree/2da6aff87b7b6c0d2321ae7f6d6bd799bbb7f592) (starting point)
- [t0urista/master](https://github.com/t0urista/esphome_component_bthome) @ [fb9d9f24fb](https://github.com/t0urista/esphome_component_bthome/tree/fb9d9f24fbe901a301b21e5631a58bba9cf4187b)
  - == open PR: [afarago#20](https://github.com/afarago/esphome_component_bthome/pull/20)
  - additional event type: button-hold
  - many additional measurement types
  - compatibility with ESP-IDF framework by `ifdef`-ing pgmspace.h
  - fix array offset math in bthome_ble_receiver_hub.cpp
- [rrooggiieerr/master](https://github.com/rrooggiieerr/esphome-bthome) @ [f5b8824518](https://github.com/rrooggiieerr/esphome-bthome/tree/f5b88245189c929fcff84a372978a0a10ba26b6c)
  - simply a merge of t0urista's work
- [Kimotu/master](https://github.com/Kimotu/esphome_component_bthome) @ [0f3d878bc8](https://github.com/Kimotu/esphome_component_bthome/tree/0f3d878bc85108fd29f39360a6764630e61f4d64)
  - beethowen docu typos
- [Kimotu/patch-2](https://github.com/Kimotu/esphome_component_bthome/tree/patch-2) @ [e07db26ee1](https://github.com/Kimotu/esphome_component_bthome/tree/e07db26ee1af35a659d64a49c230218acf797151)
  - more beethowen docu typos, not part of their master
- [digaus/master](https://github.com/digaus/esphome_component_bthome) @ [1cb9115e6f](https://github.com/digaus/esphome_component_bthome/tree/1cb9115e6f4c14c94456092694ceed383841f0bb)
  - templateable CONF_MAC_ADDRESS
  - fixed init of bthome_measurement_record_t
- [hostcc/master](https://github.com/hostcc/esphome_component_bthome) @ [0503da0484](https://github.com/hostcc/esphome_component_bthome/tree/0503da0484624b36bcb6af80a14b9c80cb5c26e4)
  - same fixed init of bthome_measurement_record_t as digaus
- [ChuckMorrisGit/master](https://github.com/ChuckMorrisGit/esphome_component_bthome) @ [95e8b8e6bd](https://github.com/ChuckMorrisGit/esphome_component_bthome/tree/95e8b8e6bdbea7a17b1fab76cb241a0346f649a4)
  - esp-idf compat by removing (not-ifdef'inf) pgmspace; conflict resolved in favour of ifdef
  - additional event type: button-hold
  - many additional measurement types (small conflict in embedded non-breaking-spaces vs normal spaces)
  - same fixed init of bthome_measurement_record_t as digaus


This author is not a competent C-programmer, but can review patches and diffs.
My intention is to play with this in a personal project (direct EspHome lighting control, no HomeAssistant server).
I hope that making my fork-overview public will help others bring the codebase forward.
I am interested in communicating with others using this codebase and syncing work to the maximum extent possible,
but make no commitment to build any kind of "community", nor am I interested in upstreaming this component to EspHome-proper.


As the original Afarago said:
If you feel the energy to take over, kindly fork and bring this forward!

---

# BTHome and Beethowen Custom Components for ESPHome

This package implements ESPHome components to low-code support Bluetooth Low Eneregy BTHome device 
standalone reception, and a leverage this protocol over ad-hoc routerless WiFi transmission over 
ESP-NOW channel.

## About
This repository contains external components for [ESPHome](https://esphome.io/).

## Installation
The installation procedure for external components is described [here](https://esphome.io/components/external_components.html).

## External components provided by this repository

- **[bthome_ble_receiver](components/docs/bthome_ble_receiver.rst)**

    This external component implements Bluetooth Low Energy BTHome local reception and decoding 
    without the need of a central hub.
    
    See also: [BTHome protocol](https://bthome.io/)

- **[beethowen_receiver](components/docs/beethowen_receiver.rst)**

    This external component implements BTHome over ESP-NOW local reception and decoding 
    without the need of a central hub.
    
    See also: [BTHome protocol](https://bthome.io/) over [ESP-NOW communication protocol](https://www.espressif.com/en/solutions/low-power-solutions/esp-now)

- **[beethowen_transmitter](components/docs/beethowen_transmitter.rst)**

    This external component implements BTHome over ESP-NOW local encoding and transmission 
    for low energy smart devices.

    See also: [BTHome protocol](https://bthome.io/) over [ESP-NOW communication protocol](https://www.espressif.com/en/solutions/low-power-solutions/esp-now)

## Overview
![Overview](components/docs/bthome_and_beethowen.drawio.png) 
