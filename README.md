# Maintenance Note

The original [afarago upstream](https://github.com/afarago/esphome_component_bthome) notes that the project has been archived, and the author no longer has interest in maintaining it.  
Since then, several forks have made various minor (and less minor) improvements to the codebase.

This repo represents a review of all those (public) forks, and attempts to provide a new unified starting point.
For details on how I performed this process, and an overview of what was merged, please see the [Ecosystem Review document](misc/ecosystem_review.md)

This author is not a competent C-programmer, but can review patches and diffs.
My intention is to play with this in a personal project (direct EspHome lighting control, no HomeAssistant server).
I hope that making my fork-overview public will help others bring the codebase forward.
I am interested in communicating with others using this codebase and syncing work to the maximum extent possible,
but make no commitment to build any kind of "community", nor am I interested in upstreaming this component to EspHome-proper.


As the original Afarago said:
If you feel the energy to take over, kindly fork and bring this forward!

---

# BTHome and Beethowen Custom Components for ESPHome

This package implements ESPHome components to low-code support Bluetooth Low Energy BTHome device
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

## Internal modules

- **[beethowen_base]()**
- bthome_base
- bthome_receiver_base

## Overview
![Overview](components/docs/bthome_and_beethowen.drawio.png) 
