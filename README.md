# The Arduino SD Game Boy Printer

![Printing example](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Print_test2.png)

# Print everything you want with your Game Boy Printer from SD card !

Why this code ? I've tried many other codes aiming to use Arduino to take control of the printer without been able to do anything, even just compiling them. I must admit that I'm not particularly gifted in C, nor patient. I would not waste a day just trying to make another project working. But why not wasting a day to do something that works easily ?

This project provides an easy solution to hack the Game Boy Printer. You just need to load a file on SD card and print (after some wiring) ! The code is divided into two parts : a converter ran with Octave/Matlab to transform any image that fits the format of a GameBoy printed image (160 pixels width, multiple of 16 pixel height, 4 shades of gray) into a tile format, and an arduino code that interprets this tile format into Game Boy Printer protocol and sends it to the printer. The code may appear gross for any professionnal programmer but hey, it works at least !

This project have its own counterpart : how to print pixelated faded out images without the Game Boy printer :
https://github.com/mofosyne/GameboyPrinterPaperSimulation


# How to use it

First you have to use prepare an SD card formatted in FAT32 with cluster size of at least 4096 ko (less will lead to printing artifacts like missing lines, for reasons explained later). Then choose an image to convert. Simply modify this line in Image_converter2.m to link to the image :

    a=imread('GB.png');

You can use any image format that do not use destructive compression, so jpg is not an option (png recommanded). Then run this code with Octave/Matlab. The code will translate the image into tiles then into Game Boy tile format in hexadecimal. Any other tool doing this can work. I choose Octave, an open source code, for the ease of programming. Details of the image to tile transformation which is a bit tricky are exposed here for example :
https://blog.flozz.fr/2018/11/19/developpement-gameboy-5-creer-des-tilesets/

Octave/Matlab converter so generates the file Hex_data.txt which contains the tiles encoded in hexadecimal. This file must then be loaded on SD card, on your SD shield. It will then be interpreted as a Game Boy Printer protocol by the Arduino code. If you open Hex_data.txt in a text editor, you will remark lots of "B" before hexadecimal data. These character are just used to match the data packet size of the Game Boy printer with the size of a multiple of 4096 bytes to avoid any 'stall' when reading the data on SD card. The timing is in fact critical. If a data packet merges on two clusters, the few milliseconds needed to pass from one cluster to the other ruins the protocol and the printer rejects the packet. 

You can change the name of the text exchange file here (if necessary, but it is not) in Game_Boy_SD_printer3.ino :

    File dataFile2 = SD.open("Hex_data.txt");

Example of Game Boy Printer protocol can be found here : 
https://gbdev.gg8.se/wiki/articles/Gameboy_Printer

The printing starts automatically once the Arduino is powered, so connect the Arduino to the Game Boy Printer and switch the printer on first. Rebooting the Arduino causes another print. You can of course directly print images from Game Boy Camera as they have natively the good format. You can start from the images extracted with this code for example :
https://github.com/mofosyne/arduino-gameboy-printer-emulator

So this tool allows you to print digital backups of Game Boy Camera images, among other things. The length of printed image could be as long as your paper roll as soon as the width is 160 pixels and your batteries full charge. The code do not take in charge multiple files to print for the moment.

![Principle](https://github.com/Raphael-Boichot/The-Arduino-SD-Game-Boy-Printer/blob/master/Illustrations/How_to.png)

# The protocol used

The protocol followed by the Arduino is the following :

![Game Boy Printer Protocol](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_protocol.PNG)

The protocol is a little bit simplier than the one used classically by the Game Boy. Here nine blocks of data containing 40 tiles (2 rows of 20 tiles, 160x16 pixels) are loaded into memory before printing (less for the last packets), and the inquiry command to check if the printer is busy is just replaced by a delay (you will see the real inquiry onscreen but I did not interpret the bytes). The reason is quite simple : it is easier to code for a lazy guy like me. So during the whole protocol, I do not check other than visually in the Arduino.serial wether the printer is responding or not. It says "0x81" (10000001) in the first response byte if it is alive, and some other informations if the second byte (refer to documentation for more) that I used just for debugging. To see what the printer sends really, uncomment this line in the void printing() : 

    //Serial.print(bit_sent);

And you will see what the Arduino sends to the printer, basically just the protocol depicted here.

# The pinout

Be careful, the default pinout may vary compared to other projects (to adapt depending on your particular SD shield setting). It is very mandatory to format your SD card with at least 4096 ko clusters. The reading speed on SD, except inbetween clusters, is enough to support the amazing baudrate of Game Boy serial protocol.

![Game Boy Printer to Arduino Uno pinout](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Pinout.PNG)
![My personnal setting](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/My_setting.PNG)

# Now have fun with it !!!

![Printing a Game Boy Camera image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example2.PNG)
![Printing a random image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example.PNG)
![Printing a custom banner](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example3.PNG)
