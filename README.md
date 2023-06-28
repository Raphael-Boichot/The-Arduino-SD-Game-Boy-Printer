# The Arduino SD Game Boy Printer

![Printing example](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Print_test2.png)

# Print everything you want with your Game Boy Printer from SD card !

Why ? I've tried many other codes aiming to use Arduino to take control of the printer without been able to do anything. So let's waste a day to do something that works !

This project provides an easy solution to hack the Game Boy Printer. You just need to load a file on SD card and print (after some wiring) ! The code is divided into two parts : a converter ran with Octave/Matlab to encode any batch of images that fits the format of a GameBoy printed image (160 pixels width, 4 shades of gray or less, multiple of 16 pixels heigth) into a tile format, and an arduino code that interprets this tile format into Game Boy Printer protocol and sends it to the printer, from an SD card.

This project have its own counterpart, [how to print images out of your Game Boy games without the Game Boy printer](https://github.com/mofosyne/GameboyPrinterPaperSimulation). The idea comes from the [Game Boy Camera Club discord](https://disboard.org/nl/server/568464159050694666).

# How to use it

First you have to prepare an SD card formatted in FAT32, old slow cards may work. Then choose a batch of images to convert (160 pixels width, 4 shades of gray, multiple of 16 pixels heigth). If you start from an unspecified color image, it is recommanded to crop the image (if necessary), resize, and apply a 4 levels grayscale (or less) with dithering. Xnview can do this task in few clics for example. Then simply drop the images into the folder ./Image_converter_3/Images. 

You must use .PNG format so that the image are red by the tile encoder. Then run Image_converter_3.m with Octave/Matlab. The code will translate images into Game Boy tile format, in hexadecimal. Images will be separated by a margin of 3 lines by default (can be changed in the image converter). Any other tool doing this can work. I choose Octave, an open source multi-platform software, for the ease of programming. Details of the image to tile transformation which is a bit tricky are exposed [here for example](https://blog.flozz.fr/2018/11/19/developpement-gameboy-5-creer-des-tilesets/).

Octave/Matlab converter so generates the file Hex_data.txt which contains the tiles encoded in hexadecimal for all images. This file must then be loaded on SD card, on your SD shield. It will then be interpreted as a Game Boy Printer protocol by the Arduino code. You can change the name of the text exchange file here (if necessary, but it is not) in Game_Boy_SD_printer_5_buffer.ino :

    File dataFile2 = SD.open("Hex_data.txt");

Explanations about the Game Boy Printer protocol can be found [here](https://gbdev.gg8.se/wiki/articles/Gameboy_Printer), [here](http://furrtek.free.fr/?a=gbprinter) or [here](https://www.mikrocontroller.net/attachment/34801/gb-printer.txt).

The printing starts automatically once the Arduino is powered, so connect the Arduino to the Game Boy Printer and switch the printer on first. Rebooting the Arduino causes another print. You can of course directly encode images from Game Boy Camera as they have natively the good format. You can start from the images extracted with the [Arduino Game Boy Printer Emulator](https://github.com/mofosyne/arduino-gameboy-printer-emulator).

So this tool allows you to print digital backups of Game Boy Camera images, among other things. The length of printed image could be as long as your paper roll as soon as the width is 160 pixels and your batteries full charge. The code can print in a same batch as many images as the SD card can handle in tile format.

# Summary

 0. Create images 160 pixels width, multiple of 16 pixel height, 4 shades of gray (or less) with dithering;
 1. OR simply pick digital Game Boy images printed from Camera or any game (must be pixel perfect, 4 shades of gray);
 2. Convert images in Game Boy tile format with Octave/Matlab by simply dropping them in a folder and running the converter;
 3. Copy the Hex_data.txt generated on an SD card formatted in FAT32, transfer card to the SD shield;
 4. Plug the Arduino to Game Boy Printer, Power the printer, power the Arduino;
 5. Enjoy the print !
 6. For another print, simply reboot the Arduino, for another image, repeat from step 0. or 1.

 By opening the Arduino.serial at 115200 bauds, you can track the protocol used, from Arduino or Game Boy printer point of view, during the whole printing process.

The image converter simply rejects images with less than 2 or more than 4 colors or grayscales or width non equal to 160 pixels. Heigth not multiple of 16 pixels are fixed on the fly by adding rows of white pixels to keep the aspect ratio of the original image. Non PNG images are just ignored.

![Principle](https://github.com/Raphael-Boichot/The-Arduino-SD-Game-Boy-Printer/blob/master/Illustrations/How_to.png)

# The protocol used

The protocol coded into the Arduino is the following :

![Game Boy Printer Protocol](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_protocol.png)

The protocol is a little bit simplier than the one used classically by the Game Boy. 9 blocks of data containing 40 tiles (2 rows of 20 tiles, 160x16 pixels) are loaded into the 8 ko internal memory before printing (less for the last remaining packets), one after the other. The inquiry command to check if the printer is busy is just replaced by a 1200xnumber of packets delay (ms), approximate time to print data stored in printer memory. I call the inquiry command every 1200 ms, but for fun. The protocol is fully open loop, the Arduino does not mind wether the serial cable is connected or not). The response bytes of the printer are anyway captured. Printer says "0x81" (10000001) in the first response byte if it is alive, and some other informations in the second byte. To see what the printer really says, or what the Arduino realy sends, change the value on this line in the Arduino code : 

    int mode=1; //1:prints Arduino->Printer communication  2:prints Printer->Arduino communication  3:minimal serial output (faster)
    
Globally, the code is very optimized to allow buffering of one data packet into the tiny Arduino Uno memory. I cannot add more live comment or additionnal feature without impeding the stability. I did not use the margin option of the print command, I rather fill the Hex_data.txt file with 3 blanck packets between each image. It allows you to visualize the limit between images in the Game Boy tile formatted data and it allows me to just send raw packets on the SD card without dedicated extra commands to separate the images.

# The pinout

Be careful, the default pinout may vary compared to other projects (to adapt depending on your particular SD shield setting).

![Game Boy Printer to Arduino Uno pinout](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Pinout.PNG)

# Some technical facts

Most of the printers comes with a Toshiba TMP87CM40AF 8-bits MCU that contains 8 kbytes rom data (among them the internal code for interpreting data, the "hello" banner, perhaps some undocumented commands, etc.) and 1024 bytes of internal ram extended by an additionnal 8192 bytes ram chip. Surprisingly, only 9 packets of data can be sent in a printing batch, which means that between 2432 and 3456 bytes of ram are reserved by the printer sytematically. This additionnal ram is probably used to buffer decoded pixel intensity sent to the printer head, packets by packets or as a whole, as printer knows the palette only at the end of serial transmission. The printer also has an early variant motherboard with a NEC MCU (the rom data are probably the same than the ones of Toshiba), 512 bytes of internal ram and 8 kbytes of ram extension but this custom NEC microcontroller is very poorly documented. I've added some documentation about the Toshiba MCU into the repo as well as a dump of the TMP87CM40AF of a dead Game Boy Printer made by crizzlycruz from Game Boy Camera Club.

Some other informations : there is a maximum of 1.49 ms delay allowed between bytes into a packet. If the delay between two bytes is longer than this, the whole packet is simply rejected. This is why the code does not read directly from SD card and use a data packet buffer. Reading directly to SD card and sending bytes is too unstable to ensure that the critical delay of 1.49 ms between bytes is respected. The instability is due to the stalling of the reading process on SD card when jumping from one sector to another. I've made the direct reading on SD card possible in early versions of the code by forcing the file format (with the image decoder) to be one data packet per sector (stuffing SD card sectors with garbage bytes after each data packet so that a packet exactly matched the sector size). Honestly, it is a bit overkill for this kind of tool. It would also oblige the user to use a specific sector size when formating SD card. So I implemented the buffering, more generic, more stable. 

Funfact, this project is compatible with the [NeoGB Printer](https://github.com/zenaro147/NeoGB-Printer), which means that you can fake a print on a fake printer, which is the most convoluted way of transmitting a 2bpp image.

The INIT command is valid at least 10 seconds, but the other packets themselves have a shorter lifespan of about 110 ms if they are not followed by another packet. I used this 110 ms delay to read on the SD card and fill the data packet buffer in Arduino memory. Hopefully the process of buffering is short enough to stay below the lifespan of preceding packet.

When using the GB printer with actual games, the palette can be freely chosen to match the 2-bits encoded grayscale value (between 0 and 3) with the 2-bits darkness levels (0 = white, 3 = black). Games can even in some case use a two or three colors palette (on purpose) or a different palette at each print command. Palette 0xE4 (the mainly encountered palette in games) is always used here for convenience.

Concerning the printing intensity, I use the default printing intensity of 0x40 (0x80 to 0xFF are also default), but you can use darker or softer print by commenting/decomenting these lines :

    //byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x7F,0x6A,0x01,0x00,0x00}; //PRINT without feed lines, darker
    byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x40,0x2B,0x01,0x00,0x00}; //PRINT without feed lines, default
    //byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x00,0xEB,0x00,0x00,0x00}; //PRINT without feed lines, lighter

Be carefull, for each byte you will modify to play with commands, you also have to change the checksum (LSB first !).

# Unexpected properties of the 0F and 04 commands

By messing with the printer protocol, I've discovered two things that are not clearly indicated into the Game Boy programming manual:

- *On one hand, INQUIRY packet (command 0x0F) systematically resets the status bit "unprocessed data" of the printer to 0, whatever the moment it is called (before or after printing). This should indicate that real games do not mind the status of the printer concerning this particular error bit as 0x0F command can be called anytime in the protocol.*
- *On the other hand, an empty DATA packet (command 0x04 with 0 load) systematically set the status bit "image data full" to 1, which seems to be a mandatory triggering interrupt for printing, whatever the state of the other status bits or printer memory filling (1 to 9 data packet stored in memory leads to the same status). I think that "image data ready" or "end of transmission" would be more appropriate than "image data full" as bit name...*

These particularities should be included in any printer emulator to ensure a 100% compatibilty with games.

# Where to buy 38 mm thermal paper for the Game Boy Printer ?

I do not recommend cutting wider roll of thermal paper (risk of frequent paper jam, crappy result) or buying outdated old Nintendo stocks as the results will be deceptive (faint printing on yellowish paper). Any fresh 38 mm wide thermal paper will do the job. In Europe, [Quorion](https://www.quorion.com/products/accesories/receipt-rolls/) produces 38 mm thermal paper and sells via Amazon (found by @R.A.Helllord from the Game Boy Camera Club Discord). [Tillrollking](https://www.tillrollking.co.uk/thermal-rolls) sells 38 mm thermal paper roll (seen on [Reddit](https://www.reddit.com/r/Gameboy/comments/d2sq77/game_boy_printer_paper_alternative/)). [Nakagawa Manufacturing](https://www.onlinecomponents.com/en/nakagawa-manufacturing/nap0038006-12002055.html) also produced the NAP-0038-006 thermal paper which is 38 mm wide. **In asia, very good results could be obtained with 38 mm [SEIKO S-951 thermal paper](https://mignon.hateblo.jp/entry/2021/07/01/003119).** This paper is used for the professional Stop Watch series sold by the same company. This paper could be obtained from Japan for cheap if you have a local contact or from western suppliers of sport equipments for a shameful price, but hey, science has no price ! Just remind that Seiko was the main manufacturer of the Game Boy printer head. I did not find any cheap chinese supplier for the moment. It seems that the 38 mm thermal paper is used by taxi cashier machine also.

Last but not least, Europe banned BPA (Bisphenol A) from thermal paper for good reasons, so avoid dispersing this kind of persistant chemical by buying BPA free paper. 
In any case, for making quick test, development, hacks, etc., some cropped used cashier tickets do the job.

# Now have fun with it !

![Printing a Game Boy Camera image](https://github.com/Raphael-Boichot/The-Arduino-SD-Game-Boy-Printer/blob/master/Illustrations/Printing_Examples.png)
