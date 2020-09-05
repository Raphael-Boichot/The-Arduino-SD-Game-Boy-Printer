# The Arduino SD Game Boy Printer

![Printing example](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Print_test2.png)

# Print everything you want with your Game Boy Printer from SD card !

Why this code ? I've tried many other codes aiming to use Arduino to take control of the printer without been able to do anything. I must admit that I'm not particularly gifted in C, nor patient. I would not waste a day just trying to make another project working. But why not wasting a day to do something that works easily ?

This project provides an easy solution to hack the Game Boy Printer. You just need to load a file on SD card and print (after some wiring) ! The code is divided into two parts : a converter ran with Octave/Matlab to encode any batch of images that fits the format of a GameBoy printed image (160 pixels width, multiple of 16 pixel height, 4 shades of gray) into a tile format, and an arduino code that interprets this tile format into Game Boy Printer protocol and sends it to the printer, from an SD card.

This project have its own counterpart : how to print pixelated faded out images without the Game Boy printer :
https://github.com/mofosyne/GameboyPrinterPaperSimulation


# How to use it

First you have to prepare an SD card formatted in FAT32 with cluster size of at least 2048 kbytes (less will lead to printing artifacts like missing lines, for reasons explained later). Then choose a batch of images to convert (160 pixels width, multiple of 16 pixel height, 4 shades of gray). If you start from an unspecified color image, it is recommanded to crop the image (if necessary), resize, and apply a 4 level grayscale with dithering. Xnview can do this task in few clics for example. Then simply drop the images into the folder ./Image_converter 2/Images. 

You must use .PNG format so that the image are red by the tile encoder. Then run Image_converter2.m with Octave/Matlab. The code will translate images into Game Boy tile format, in hexadecimal. Images will be separated by a margin of 3 lines by default (can be changed in the image converter). Any other tool doing this can work. I choose Octave, an open source multi-platform software, for the ease of programming. Details of the image to tile transformation which is a bit tricky are exposed here for example :
https://blog.flozz.fr/2018/11/19/developpement-gameboy-5-creer-des-tilesets/

Octave/Matlab converter so generates the file Hex_data.txt which contains the tiles encoded in hexadecimal for all images. This file must then be loaded on SD card, on your SD shield. It will then be interpreted as a Game Boy Printer protocol by the Arduino code. If you open Hex_data.txt in a text editor, you will remark a header of junk data before each data packet. These characters are just used to match the data packet size with a multiple of 2048 bytes to avoid any 'stall' when reading the data on SD card. The timing is in fact critical. If a data packet merges on two clusters, the few milliseconds needed to pass from one cluster to the other ruins the protocol and the printer rejects the packet. Indeed the SD shield library seems to not include any reading buffer. You can change the name of the text exchange file here (if necessary, but it is not) in Game_Boy_SD_printer3.ino :

    File dataFile2 = SD.open("Hex_data.txt");

Explanations about the Game Boy Printer protocol can be found here (among other sources) : 
https://gbdev.gg8.se/wiki/articles/Gameboy_Printer
http://furrtek.free.fr/?a=gbprinter
https://www.mikrocontroller.net/attachment/34801/gb-printer.txt

The printing starts automatically once the Arduino is powered, so connect the Arduino to the Game Boy Printer and switch the printer on first. Rebooting the Arduino causes another print. You can of course directly encode images from Game Boy Camera as they have natively the good format. You can start from the images extracted with this code for example :
https://github.com/mofosyne/arduino-gameboy-printer-emulator

So this tool allows you to print digital backups of Game Boy Camera images, among other things. The length of printed image could be as long as your paper roll as soon as the width is 160 pixels and your batteries full charge. The code can print in a same batch as many images as the SD card can handle in tile format.

# Summary

 0. Create images 160 pixels width, multiple of 16 pixel height, 4 shades of gray with dithering ;
 1. OR simply pick digital Game Boy images printed from Camera or any game (must be pixel perfect, 4 shades of gray) ;
 2. Convert images in Game Boy tile format with Octave/Matlab by simply dropping them in a folder and running the converter ;
 3. Copy the Hex_data.txt generated on an SD card formatted in FAT32 with 2048 kbytes cluster size ;
 4. Plug the Arduino to Game Boy Printer, Power the printer, power the Arduino ;
 5. Enjoy the print !
 6. For another print, simply reboot the Arduino, for another image, repeat from step 0. or 1.

![Principle](https://github.com/Raphael-Boichot/The-Arduino-SD-Game-Boy-Printer/blob/master/Illustrations/How_to.png)

# The protocol used

The protocol followed by the Arduino is the following :

![Game Boy Printer Protocol](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_protocol.PNG)

The protocol is a little bit simplier than the one used classically by the Game Boy. 9 blocks of data containing 40 tiles (2 rows of 20 tiles, 160x16 pixels) are loaded into the 8 ko internal memory before printing (less for the last remaining packets), one after the other. The inquiry command to check if the printer is busy is just replaced by a delay (you will see the real inquiry onscreen with error codes). So during the whole protocol, I do not check other than visually in the Arduino.serial wether the printer is responding or not. It says "0x81" (10000001) in the first response byte if it is alive, and some other informations if the second byte. To see what the printer really says, or what the Arduino realy sends, comment/uncomment this line in the Arduino code : 

     //int mode=1; //prints Arduino->Printer communication
     //int mode=2; //prints Printer->Arduino communication
     int mode=3; //minimal serial output (faster)

# The pinout

Be careful, the default pinout may vary compared to other projects (to adapt depending on your particular SD shield setting). It is very mandatory to format your SD card with at least 2048 ko clusters. The reading speed on SD, except inbetween clusters, is enough to support the sluggish baudrate of Game Boy serial protocol.

![Game Boy Printer to Arduino Uno pinout](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Pinout.PNG)

# Some technical facts

Most of the printers comes with a Toshiba TMP87CM40AF MCU that contains 32 kbytes rom data (among them the internal code for interpreting data, the "hello" banner, perhaps some undocumented commands, etc. and probably lot of blank data) and 512 bytes of ram extended by an additionnal 8192 bytes ram chip. Surprisingly, only 9 packets of data can be send in a batch, which means that between 2432 and 2944 bytes or ram are reserved sytematically. This additionnal ram can be used for decompressing data on the fly (the two Pokémon Trading card games uses compression in the printing protocol). This corresponds to about three complete data packets that can be stored apart from the other packets. This additionnal ram is typically what lacks to the Arduino Uno to completely simulate a Game Boy printer. 
Some other information : there is a maximum of 1.49 ms delay allowed between bytes into a packet. If the delay between two bytes is higher than this, the whole packet is simply rejected. The INIT command is valid at least 10 seconds, but the other packets themselves have a lifespan of about 110 ms if they are not followed by another packet.  

# Now have fun with it !!!

![Printing a Game Boy Camera image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example2.PNG)
![Printing a random image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example.PNG)
![Printing a custom banner](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example3.PNG)
