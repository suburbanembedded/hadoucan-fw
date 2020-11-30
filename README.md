# Hadou-CAN FW

## What is it?

The Hadou-CAN is a USB to CAN FD adapter supporting the newest CAN standard, CAN FD. It uses the standard Lawicel text mode protocol and is compatible with slcand on Linux.

The electrically isolated CAN side protects your computer in high power robotics applications, in distributed networks where the ground offset may be large, or anywhere common mode noise is significant.

The first-in-class PPS timesync feature allows any number of Hadou-CAN to synchronize the timestamps on messages from separate, isolated CAN buses. A Hadou-CAN may be initialized as a master source of the PPS sync signal, or as a slave to receive sync signals and track the master clock. Incoming packets are automatically tagged with the synchonized timestamps, so that debugging large systems with multiple, isolated CAN buses has never been easier.

## Supported Platforms
* Linux
* Windows

## What makes it special?
* CAN FD support at 12 Mbps
* A timesync input/output allows timestamps to match between several networks 
* Electrical isolation between the USB and CAN bus sides to protect your equipment
* Bootloader with support for loading custom user programs
* Blue extruded Al case option
* Jumpers to add and remove termination resistors
* Automatic slew rate control for low EMI or high speed

## Connectivity
* USB: type B, USB 2.0 device
* CAN: DE-9
* Timesync: SMA, 3.3V 50 ohm

## Features coming soon
* HW ID filters & masks
* Enhancements to the USB stack

## Build Status
### Master Branch
![Hadou-CAN CI](https://github.com/suburbanembedded/hadoucan-fw/workflows/Hadou-CAN%20CI/badge.svg?branch=master)

## Documentation

User Guide
https://docs.suburbanmarine.io/public/hadoucan/doc/Hadou-CAN_User_Guide.pdf

Datasheet
https://docs.suburbanmarine.io/public/hadoucan/doc/Hadou-CAN_Datasheet.pdf

Quickstart Guide
https://docs.suburbanmarine.io/public/hadoucan/doc/Hadou-CAN_Quickstart_Guide.pdf

## Store

https://www.tindie.com/products/suburbanembedded/hadou-can-usb-can-fd-adapter/

https://www.digikey.com/product-detail/en/suburban-embedded/SM-1301-C01-ENC/2587-SM-1301-C01-ENC-ND/12149330

## Copyright

Copyright (c) 2018-2020 Suburban Embedded

## License

Licensed under the terms of the 3-Clause BSD license. See LICENSE for details.
