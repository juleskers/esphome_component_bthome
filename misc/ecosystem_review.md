# Ecosystem Review

This author tries to keep the entire fork network merged into their upstream, in hopes of providing a unified
codebase for the community to work on.

This document serves as documentation (and note-to-self) on how to do this.


## Steps to perform a review

1. Keep an eye on the ["forks / network members"](https://github.com/afarago/esphome_component_bthome/network/members) 
   page on GitHub, to discover new forks (or forks-of-forks-of....)
2. Ensure all forks are added as remotes to local clone.  
   ```shell
   for USERNAME in \
     juleskers DanCech Kimotu MrSuicideParrot afarago digaus edestecd evlo hostcc hqsone iChenlife jerome83136 konistehrad mathgaming rrooggiieerr t0urista unsplorer \
     ; do 
       git remote add $USERNAME git@github.com:$USERNAME/esphome_component_bthome.git; 
     done;
   ```
3. `git fetch --all --prune`
4. Update local branch to latest public upstream.  
   `git checkout main`  
   `git merge --ff-only juleskers:main`
6. Look for dangling (not yet merged into main) branches anywhere in the known remotes:  
   `git branch --remote --all --no-merged`  
   (shows any branch, including remotes, not yet merged into current, sometimes git is just really powerful...)
7. If anything new has shown up, start a review-branch, from current main  
   `git switch -c ecosystem-review juleskers:main`
8. Merge new stuff  
   ....
9. When finished, merge back, with an explicit merge commit to document the date of the review.  
   ```shell
   git checkout main
   git merge --no-ff ecosystem-review
   git push juleskers
   ```


## What has been reviewed so far

Summarizing the kind of work that was done, and where (sometimes the same in multiple spots)

### 2025-08-19

7 forks with changes, not counting my own.

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
    - same fixed init of `bthome_measurement_record_t` as digaus
- [ChuckMorrisGit/master](https://github.com/ChuckMorrisGit/esphome_component_bthome) @ [95e8b8e6bd](https://github.com/ChuckMorrisGit/esphome_component_bthome/tree/95e8b8e6bdbea7a17b1fab76cb241a0346f649a4)
    - esp-idf compat by removing (not-ifdef'inf) pgmspace; conflict resolved in favour of ifdef
    - additional event type: button-hold
    - many additional measurement types (small conflict in embedded non-breaking-spaces vs normal spaces, kept NBSP)
    - same fixed init of `bthome_measurement_record_t` as digaus

### 2026-01-25

- [rrooggiieerr/master](https://github.com/rrooggiieerr/esphome-bthome) @ [1397b1ad20](https://github.com/rrooggiieerr/esphome-bthome/tree/1397b1ad205589dfeff47775cf5669159f7cd7d4)
  - Manually removed duplicated generated keys in `const_generated.py`.
  - This is needed because `generate_bthome_constants.py` generates conflicting short-names for several newly added
    measurement types (variants of pre-existing ones, such as `temperature` or `count`).  
  - See [tracking issue #1](https://github.com/juleskers/esphome_component_bthome/issues/1) for more on this topic.
- [edestecd/HEAD](https://github.com/edestecd/esphome-bthome) @ [a6078966f3](https://github.com/rrooggiieerr/esphome-bthome/tree/a6078966f3268ad9e71c7eef14c9916b9f21a206)
  - yet another fixing the `bthome_measurement_record_t` init. Semantically same as digaus
  - at least documents the reason: C++20 correctness
- AFarago added funding links for himself, which I took the liberty of not copying for my clone.

### 2026-02-04

- [unsplorer/master](https://github.com/unsplorer/esphome-bthome) @ [55ecc6aa8843](https://github.com/unsplorer/esphome-bthome/tree/55ecc6aa884314abd1ec176f38ec8f874186a7b2)
  - Fixed a method signature, which, due to differences in ESP-IDF API version used between ESP8266 and ESP32, wouldn't
    compile for ESP8266 (anymore, after ESP updated IDF for *only* ESP32).
  - See details in [#4: Understand, review and merge remote: unsplorer](https://github.com/juleskers/esphome_component_bthome/issues/4)