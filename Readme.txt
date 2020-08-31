By Raphaël BOICHOT 2020/08/31

Hardware 
- Arduino Uno
- Whatever SD shield compatible
- Custom cable to link your arduino to the GB printer. Fake chinese cables recommanded.

Pinout : 
- Clock = Arduino D2
- TX = Arduino D3
- RX = Arduino D5
- CS from SD shield = Arduino D4 (depends on your SD shield).
- Recent (2020) SDHC card recommanded for stability of reading speed (old cards may not work reliably).

1) Choose an image and modify it so that it is only 4 shades of gray, 160 pixels wide, heigth multiple of 16 pixels. You can choose a pixel perfect image from your Game Boy Camera or other games using the Game Boy Printer also.
2) Transform it into Game Boy tile format with the Octave/Matlab code provided
3) Upload to an SD card with the default name "Hex_data.txt"
4) Plug your Arduino to the Game boy printer, power it, power Arduino and enjoy the print !
5) For a second print or another one reloaded on SD, just reboot Arduino.