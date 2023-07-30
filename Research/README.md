This code is just a serial interface between any device (PC, phone app, etc.) and a Game Boy Printer. It does nothing else than reading a byte in the serial, bitbang the printer at the good rate, sent back the response byte of the printer, and so on. You have to deal with everything else.

**Beware, the Octave code does not work as expected for the moment ! It runs with very poor synchronisation for reasons I do not fully understand**

