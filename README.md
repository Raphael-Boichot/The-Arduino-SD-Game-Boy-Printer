# The Arduino SD Game Boy Printer

![Printing example](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Print_test2.png)

# Print everything you want with your Game Boy Printer from SD card !

By Raphaël BOICHOT, september 2020. Last update: 2021-08-01.

Why this code ? I've tried many other codes aiming to use Arduino to take control of the printer without been able to do anything. I must admit that I'm not particularly gifted in C, nor patient. I would not waste a day just trying to make another project working. But why not wasting a day to do something that works easily ?

This project provides an easy solution to hack the Game Boy Printer. You just need to load a file on SD card and print (after some wiring) ! The code is divided into two parts : a converter ran with Octave/Matlab to encode any batch of images that fits the format of a GameBoy printed image (160 pixels width, multiple of 16 pixel height, 4 shades of gray) into a tile format, and an arduino code that interprets this tile format into Game Boy Printer protocol and sends it to the printer, from an SD card.

This project have its own counterpart : how to print pixelated faded out images without the Game Boy printer :

https://github.com/mofosyne/GameboyPrinterPaperSimulation

The idea comes from the Game Boy Camera Club discord : 

https://disboard.org/nl/server/568464159050694666

# How to use it

First you have to prepare an SD card formatted in FAT32, old slow cards may work. Then choose a batch of images to convert (160 pixels width, multiple of 16 pixel height, 4 shades of gray). If you start from an unspecified color image, it is recommanded to crop the image (if necessary), resize, and apply a 4 level grayscale with dithering. Xnview can do this task in few clics for example. Then simply drop the images into the folder ./Image_converter_3/Images. 

You must use .PNG format so that the image are red by the tile encoder. Then run Image_converter_3.m with Octave/Matlab. The code will translate images into Game Boy tile format, in hexadecimal. Images will be separated by a margin of 3 lines by default (can be changed in the image converter). Any other tool doing this can work. I choose Octave, an open source multi-platform software, for the ease of programming. Details of the image to tile transformation which is a bit tricky are exposed here for example :

https://blog.flozz.fr/2018/11/19/developpement-gameboy-5-creer-des-tilesets/

Octave/Matlab converter so generates the file Hex_data.txt which contains the tiles encoded in hexadecimal for all images. This file must then be loaded on SD card, on your SD shield. It will then be interpreted as a Game Boy Printer protocol by the Arduino code. You can change the name of the text exchange file here (if necessary, but it is not) in Game_Boy_SD_printer_5_buffer.ino :

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
 3. Copy the Hex_data.txt generated on an SD card formatted in FAT32 ;
 4. Plug the Arduino to Game Boy Printer, Power the printer, power the Arduino ;
 5. Enjoy the print !
 6. For another print, simply reboot the Arduino, for another image, repeat from step 0. or 1.

![Principle](https://github.com/Raphael-Boichot/The-Arduino-SD-Game-Boy-Printer/blob/master/Illustrations/How_to.png)

# The protocol used

The protocol followed by the Arduino is the following :

![Game Boy Printer Protocol](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_protocol.png)

The protocol is a little bit simplier than the one used classically by the Game Boy. 9 blocks of data containing 40 tiles (2 rows of 20 tiles, 160x16 pixels) are loaded into the 8 ko internal memory before printing (less for the last remaining packets), one after the other. The inquiry command to check if the printer is busy is just replaced by a delay (you will see the real inquiry onscreen with error codes). So during the whole protocol, I do not check other than visually in the Arduino.serial wether the printer is responding or not. It says "0x81" (10000001) in the first response byte if it is alive, and some other informations in the second byte. To see what the printer really says, or what the Arduino realy sends, change the value on this line in the Arduino code : 

    int mode=1; //1:prints Arduino->Printer communication  2:prints Printer->Arduino communication  3:minimal serial output (faster)
    
Globally, the code is very optimized to allow buffering of one data packet into the small Uno memory. I cannot add any live comment or additionnal feature without impeding the stability. I did not use the margin option of the print command, I rather fill the Hex_data.txt file with 3 blanck packets between each image. It allows you to visualize the limit between images in the Game Boy tile formatted data and it allows me to just send raw packets on the SD card without dedicated extra commands to separate the images.

# The pinout

Be careful, the default pinout may vary compared to other projects (to adapt depending on your particular SD shield setting).

![Game Boy Printer to Arduino Uno pinout](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Pinout.PNG)

# Some technical facts

Most of the printers comes with a Toshiba TMP87CM40AF 8-bits MCU that contains 32 kbytes rom data (among them the internal code for interpreting data, the "hello" banner, perhaps some undocumented commands, etc. and probably lot of blank data) and 1024 bytes of internal ram extended by an additionnal 8192 bytes ram chip. Surprisingly, only 9 packets of data can be sent in a printing batch, which means that between 2432 and 3456 bytes of ram are reserved by the printer sytematically. This additionnal ram can be used for decompressing data on the fly (the two Pokémon Trading card games uses compression in the printing protocol). This corresponds to about 3 to 5 complete data packets that can be stored apart from the other packets. To me, the data packets are first stored one by one in this dedicated part of the ram (coming from compressed protocol or not), validated (checksum and lifespan) and them copied into the 9 regular tracks allowed for printing. Only a disassembly of the 32 kbytes MCU eprom could confirm this. The printer also has an early variant motherboard with a NEC MCU having only 8 kbytes of rom data (the rom data are probably the same than the ones of the 32 kbytes Toshiba), 512 bytes of internal ram and 8 kbytes of ram extension but this custom NEC microcontroller is very poorly documented.

Some other informations : there is a maximum of 1.49 ms delay allowed between bytes into a packet. If the delay between two bytes is longer than this, the whole packet is simply rejected. This is why the code does not read directly from SD card and use a data packet buffer. Reading directly to SD card and sending bytes is too unstable to ensure that the critical delay of 1.49 ms between bytes is respected.

The INIT command is valid at least 10 seconds, but the other packets themselves have a shorter lifespan of about 110 ms if they are not followed by another packet. I used this 110 ms delay to read on the SD card and fill the data packet buffer in Arduino memory. Hopefully the process of buffering is short enough to stay below the lifespan of preceding packet.

When using the GB printer with actual games, the palette can be freely chosen to match the 2-bits encoded grayscale value (between 0 and 3) with the 2-bits darkness levels (0 = white, 3 = black). Games can even in some case use a two colors palette (on purpose) or a different palette at each print command. Palette 0xE4 (the mainly encountered palette in games) is always used here for convenience.

Concerning the printing intensity, I use the default printing intensity of 0x40 (0x80 to 0xFF are also default), but you can use darker or softer print by commenting/decomenting these lines :

    //byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x7F,0x6A,0x01,0x00,0x00}; //PRINT without feed lines, darker
    byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x40,0x2B,0x01,0x00,0x00}; //PRINT without feed lines, default
    //byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x00,0xEB,0x00,0x00,0x00}; //PRINT without feed lines, lighter

Be carefull, for each byte you will modify to play with commands, you also have to change the checksum (LSB first !).

# Now have fun with it !!!

![Printing a Game Boy Camera image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example2.PNG)
![Printing a random image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example.PNG)
![Printing a custom banner](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example3.PNG)
