# AgIsoStack-Arduino
AgIsoStack is a free ISOBUS (ISO11783) and SAE J1939 compatible CAN stack that makes communication on off-highway vehicle CAN networks easy.

This library is based on the larger [AgIsoStack++ project](https://github.com/Open-Agriculture/AgIsoStack-plus-plus), which provides a CMake build system and additional supported CAN hardware.

Currently this Arduino library is compatible with Teensy hardware only. ESP32 support will likely be added at some point, but for now PlatformIO + ESP-IDF is supported for ESP32 platforms via the [main repo](https://github.com/Open-Agriculture/AgIsoStack-plus-plus).

### Features

- ISO11783/J1939 Address Claiming
- ISO11783 Universal/Virtual Terminal Client
- ISO11783 Task Controller Client
- NMEA2000 Fast Packet
- J1939 and ISO11783 Diagnostic Messages
- Implement/Machine speed and guidance messaging
- ISOBUS Shortcut Button (ISB)
- ISO11783 Transport Protocol
- ISO11783 Extended Transport Protocol
- Designed to integrate with proprietary messaging as needed

### Example

Examples are located [here](https://github.com/Open-Agriculture/AgIsoStack-Arduino/tree/main/examples).
The virtual terminal example sketch is a good starting point, and loads a VT object pool to a virtual terminal, as long as your teensy is connected to an ISO11783 CAN network using the Teensy's CAN 1 pins and compatible CAN transceiver.

### Troubleshooting

* My Teensy won't start up after loading this library, or otherwise does nothing
    * There might be a bug in the Teensy core, where compiling with non-default optimization causes this issue.
    * You can fix this by editing your startup.c file for your Teensy.
    * Navigate to your teensy core files located normally at `C:\Users\<user>\AppData\Local\Arduino15\packages\teensy\hardware\avr\<version>\cores\teensy4`
    * Open `startup.c`
    * Add this line after the `#include` directives: `#pragma GCC optimize ("O2")`
* My Teensy starts fine, but no object pool is loaded!
    * Make sure you are using a [proper CAN transceiver](https://www.amazon.com/SN65HVD230-CAN-Board-Communication-Development/dp/B00KM6XMXO). You can't connect CTX1 and CRX1 directly to a CAN bus.
    * Make sure you don't have CAN-H and CAN-L reversed.
    * Make sure your CAN network is properly terminated.
    * Try and view the serial output from the Teensy to see if any errors are shown.
    * Make sure your object pool isn't in EXTMEM without first being initialized, otherwise your pool might be all zeros, or worse.
* My program doesn't compile because my Teensy is out of RAM1 space
    * Make sure you are compiling with optimizations set to "Smallest Code".
* I don't see Teensy as a board option in Arduino IDE
    * Ensure you have followed the steps [outlined here](https://www.pjrc.com/teensy/td_download.html) to add the Teensy boards.
* Something else!
    * Open an issue or start a discussion here on GitHub and a maintainer will try to help if they can.

### Documentation

View the [precompiled doxygen](https://delgrossoengineering.com/isobus-docs/) or visit our [tutorial site](https://isobus-plus-plus.readthedocs.io/en/latest/).

Note that the documentation is for the full C++ library, so file paths and other minor differences might exist on Arduino, but they should be very minor.

You may also want to view the main repo's [examples](https://github.com/Open-Agriculture/AgIsoStack-plus-plus/tree/main/examples).
