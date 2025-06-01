# Yet Another PC to Game Boy Printer interface

This project is designed to make sending data to the Game Boy Printer as simple as possible. The Arduino handles everything — checksums, timing, errors, and formatting — so you only need to send two very simple types of packets to print. An example code is provided, but any programming language can be used to control the Arduino.

## Printing Procedure

- The Arduino automatically connects to the printer and initializes it. Wait for the "Printer connected" message from the serial monitor before sending data. The initialization remains valid for at least 10 seconds.

- To send data, use the following packet format:
**["D"][640-byte [Gameboy 2BPP graphics data](https://www.huderlem.com/demos/gameboy2bpp.html)][CR]** (total of 642 bytes, CR is char(13)).
Wait for the "Printer ready" message before sending the next packet. Each packet remains valid for 150 milliseconds unless followed immediately by another. You can send up to 9 packets consecutively. The same timing rule applies to the print command.

- To print, send the following command:
**["P"][margin][palette][intensity][CR]** (total of 5 bytes, CR is char(13)).
Wait for the "Printer ready" message to confirm that printing is complete and it's safe to proceed.

## Installation

The example interface code can be ran with [GNU Octave](https://www.octave.org/). The Arduino code can be transfered easily from the [Arduino IDE](https://www.arduino.cc/en/software/).

## Pinout and hardware

The pinout is the same as this [previous project](https://github.com/Raphael-Boichot/PC-to-Game-Boy-Printer-interface). If you want something very neat, you can follow the [instructions given here](https://github.com/Raphael-Boichot/Collection-of-PCB-for-Game-Boy-Printer-Emulators?tab=readme-ov-file) to build a dedicated PCB. This PCB is also compatible with many other projects.

![Game Boy Printer to Arduino Uno pinout](Pictures/Pinout.png)
