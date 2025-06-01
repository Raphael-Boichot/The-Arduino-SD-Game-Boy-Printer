# Yet Another Game Boy Printer PC interface

This project is designed to make sending data to the Game Boy Printer as simple as possible. The Arduino handles everything—checksums, timing, errors, and formatting—so you only need to send two very simple types of packets to print. An example code is provided, but any programming language can be used to control the Arduino.

![](Research/Super_Printer_interface/Pictures/Principle.png)

## Printing Procedure

- The Arduino automatically connects to the printer and initializes it. Wait for the "Printer connected" message from the serial monitor before sending data. The initialization remains valid for at least 10 seconds.

- To send data, use the following packet format:
["D"][640-byte [Gameboy 2BPP graphics data](https://www.huderlem.com/demos/gameboy2bpp.html)][CR] (total of 642 bytes).
Wait for the "Printer ready" message before sending the next packet. Each packet remains valid for 150 milliseconds unless followed immediately by another. You can send up to 9 packets consecutively. The same timing rule applies to the print command.

- To print, send the following command:
["P"][margin][palette][intensity][CR] (total of 5 bytes).
Wait for the "Printer ready" message to confirm that printing is complete and it's safe to proceed.

## Installation

The example interface code can be ran with [GNU Octave](https://www.octave.org/). The Arduino code can be transfered easily from the [Arduino IDE](https://www.arduino.cc/en/software/).

## Pinout and hardware

The pinout is the same as this [previous project](https://github.com/Raphael-Boichot/PC-to-Game-Boy-Printer-interface).

## Parts needed

- An [Arduino Uno](https://www.aliexpress.com/item/1005002997846504.html);
- The [cheapest Game Boy serial cable you can find](https://fr.aliexpress.com/item/32698407220.html) as you will cut it. **Important note:** SIN and SOUT are crossed internally so never trust what wires you get. Use a multimeter to identify wires. Cross SIN and SOUT if the device does not work at the end;
- If you want something working first try, you can use a [serial port breakout board](https://github.com/Palmr/gb-link-cable) instead of cutting/soldering a cable.
- If you want something blinking, you can wire an aditionnal fancy LED to D13 with a 200-1000 Ohm resistor in series.

## Pinout 

![Game Boy Printer to Arduino Uno pinout](Pictures/Pinout.png)

The pinout uses only 4 wires, so it's very easy to make !

## Dedicated PCB

If you want something very neat, you can follow the [instructions given here](https://github.com/Raphael-Boichot/Collection-of-PCB-for-Game-Boy-Printer-Emulators?tab=readme-ov-file) to build a dedicated PCB. This PCB is also compatible with many other projects.






