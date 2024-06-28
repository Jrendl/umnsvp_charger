# UMNSVP Charger Control Board Firmware

This is a snapshot of the Charger code for F1.50 (The iteration of car 14 raced in ASC 2022) that I (Joe) contributed on. 
This also includes code that I mentored younger members on. This is the state that I left the charger code when I left the team



This directory contains firmware for the charger board and interfacing all necessary peripherals.

## FIles

### application.cc
* All high level logic for the charger state machine.

### bms.cc
* Encapsulates skylab code for communicating with BMS.
* (skylab is UMNSVP's custom CAN protocol wrapper)

### can2.cc
* BxCan level driver for CAN2 network.

### j1772.cc
* Hardware drivers for communicating with the EVSE.

### main.cc
* Main executable for the charger.

### pwm_driver.cc
* Hardware drivers for reading the PWM duty cycle on the control pin of the J1772 connector.

### thunderstruck.cc
* Encapsulates all communication with the thunderstruck AC/DC convertors.

### timing.cc
* Hardware timer initalization.