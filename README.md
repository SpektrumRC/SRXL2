# SRXL2

SRXL2 is a bi-directional serial communications protocol now used in Spektrum&trade; receivers. Using a standard UART in half-duplex mode, this protocol defines a standard way for the receiver to send Channel Data digitally to Smart ESCs, Flight Controllers, and other devices on the SRXL2 bus. Through a handshaking mechanism, these devices can report how often they should be polled for their Telemetry data, and can inform the master whether they support communications at 400k baud or the default of 115.2k. This enables much more responsive Telemetry data, customized to individual needs.

This open-source library code is the same code used in Spektrum&trade; receivers, and is provided here to allow the community to more easily innovate and create third-party SRXL2 compatible devices.

### Getting Started

Check out the [Examples](https://github.com/SpektrumRC/SRXL2/Examples) folder for a simplified app example to get you familiar with the basics. For a more in-depth reference, check out the [SRXL2 Specs](https://github.com/SpektrumRC/SRXL2/blob/master/Docs/SRXL2%20Specification.pdf). In there, you will find the approved electrical connector pinouts, details on how the Device ID and handshaking scheme works, and packet details.

The [Source](https://github.com/SpektrumRC/SRXL2/Source) folder contains the C11-compatible library code that can be dropped into your project, and the example [spm_srxl_config.h](https://github.com/SpektrumRC/SRXL2/blob/master/Examples/spm_srxl_config.h) file gives you a starting point to configure and customize the required callback functions to interface with your own UART routines. You'll also find options to use hardware-accelerated CRC calculations if you are using compatible STM32 F3/F7 hardware.
