# Contribution guidelines / developing on this project

This document aims to help people get started on *changing* this project, rather than just using it.

## Working with the python codebase

The python code in this project assumes python3, with very minimal added dependencies.
Most notably, the ESPhome package, for its code-generation and config-grammar libraries.
The cleanest way to get this, without potentially messing up your entire system, 
is by starting a python virtual environment ("venv") specifically for this project:

```shell
cd path/to/your/esphome_component_bthome/
# use *system* python to initialise a new venv in the "venv" subdir (conventional name)
python3 -m venv venv/   
# use *venv* (not system!) `pip` to install dependencies into the project environment.
venv/bin/pip install esphome beautifulsoup4
```

## Updating BTHome constants

Much of the usability of this project comes from the human-readable definitions for the different
sensor or event IDs ("pressure" instead of `0x04`, "long_double_press" rather than `0x3A05`).

The canonical source for these definitions is [BTHome.io/format](https://bthome.io/format/).  
This project *generates* its code from this source, rather than implementing it manually.
However, this code generation needs some manual hand-holding and review.
Therefore, you'll manually need to invoke [tools/generate_bthome_constants.py](tools/generate_bthome_constants.py),
and review its generated output, before commiting those changes that "look good".

- Human-readable documentation table:
  [components/docs/bthome_common_format_generated.rst](components/docs/bthome_common_format_generated.rst)
- Constants for the firmware (written in C) that ends up on the ESP microcontroller
  [components/bthome_base/bthome_common_generated.h](components/bthome_base/bthome_common_generated.h)
- Constants used by the python code which generates that firmware from templates.
  [components/bthome_base/const_generated.py](components/bthome_base/const_generated.py)

### Running the generator

The constant generator is a python3 program, with very minimal dependencies.
Most notably, it needs BeautifulSoup4 (for HTML scraping the bthome.io site)
and the ESPhome package (for certain interface constants).  
See above for how to initialise the venv.

```shell
cd path/to/your/esphome_component_bthome/
cd tools/
# run generator from project environment
../venv/bin/python3 generate_bthome_constants.py
# output: 
#   generating ../components/bthome_base/bthome_common_generated.h...
#   generating ../components/bthome_base/const_generated.py...
#   generating ../components/docs/bthome_common_format_generated.rst...
```

### ⚠️ human review rules for generated constants

1. DO NOT commit the generated code as-is; doing so can break downstream projects.
2. DO keep the mapping between readable aliases and IDs constant.
   - Aliases are forever! once you define one, you cannot ever change it.
     Doing so anyway breaks users' configs, since their aliases no longer line up with their existing devices.
     Thus, add, don't replace, entries.
   - For example, if a new `temperature`-variant is added, ensure it gets a *new* alias, 
     instead of "updating" (i.e. breaking) the existing `0x02 / temperature` alias.
   - This can happen because the de-duplication code in `generate_bthome_constants` isn't very smart.
   - bad example: "updating" the existing hex-alias mapping, breaking projects using the alias.
     ```diff
      "temperature_coarse": {
     -     "measurement_type": 0x45,
     -     "accuracy_decimals": 1,
     +     "measurement_type": 0x58, # NO! don't change mappings!
     +     "accuracy_decimals": 0,
          "unit_of_measurement": "°C",
          "device_class": "temperature"
      },
     ```
   - good example: appending a second, disambiguated alias:
     ```diff
      "temperature_coarse": {
          "measurement_type": 0x45,
          "accuracy_decimals": 1,
          "unit_of_measurement": "°C",
          "device_class": "temperature"
      },
     +"temperature_coarse_short": {
     +    "measurement_type": 0x58,
     +    "accuracy_decimals": 0,
     +    "unit_of_measurement": "°C",
     +    "device_class": "temperature"
     +},
     ```
3. For the above "forever" reasons, be wary of "quick-n-dirty" manual fixes
   that you want to automate "later"; 
   - If you can't get the future generation code lined up *perfectly* with today's lazy hack,
     you're breaking users.
   - When in doubt, leave it out.  
     Even Without a human-readable alias, users will still be able to handle (most) messages using
     the raw hexadecimal IDs.