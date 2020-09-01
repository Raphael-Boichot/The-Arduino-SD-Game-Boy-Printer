# The-FakePrinter

![Printing example](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Print_test2.png)

Print everything you want with your Game Boy Printer !

Why this code ? I've tried many other codes claiming to use Arduino to take control of the printer without been able to do anything, even just compiling them. I must admit that I'm not particularly gifted in C.

This project provide an easy solution to hack the Game Boy Printer. You just need an Arduino and SD shield plus some soldering. The code is divided into two parts : a converter ran with Octave/Matlab to transform any image that fits the format of a GameBoy printed image (160 pixels width, multiple of 16 pixel height, 4 shades of gray) into a tile format, and an arduino code that interprets this tile format into Game Boy Printer protocol and sends it to the printer. The code may appear gross for any professionnal programmer but hey, it works at least !

This project have its own counterpart : how to print pixelated faded out images without the Game Boy printer :
https://github.com/mofosyne/GameboyPrinterPaperSimulation


# How to use it

To choose wich file to convert, simply modify this line in Image_converter.m

    a=imread('GB.png');

Details of the image to tile transformation which is a bit tricky are exposed here for example :
https://blog.flozz.fr/2018/11/19/developpement-gameboy-5-creer-des-tilesets/

Octave converter generates the file Hex_data.txt which contains the tiles encoded in hexadecimal. This file must then be uploaded on SD card. It will then be interpreted as a Game Boy Printer protocol by the Arduino code. You can change the name of the file here (if necessary) in TheFakePrinter.ino :

    File dataFile2 = SD.open("Hex_data.txt");

Example of Game Boy Printer protocol can be found here : 
https://gbdev.gg8.se/wiki/articles/Gameboy_Printer

The printing is starts automatically once the Arduino is powered, so connect the Arduino to the Game Boy Printer and switched it on first. Rebooting the Arduino causes another print. You can also print images from Game Boy Camera or images extracted from games with this code for example :
https://github.com/mofosyne/arduino-gameboy-printer-emulator

The length of printed image could be as long as your paper roll as soon as the width is 160 pixels and your batteries full charge.

# The protocol used

The protocol followed by the Arduino is the following :

![Game Boy Printer Protocol](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_protocol.PNG)

The protocol is a little bit different from the one used classically by the Game Boy. Here each block of data containing 40 tiles (2 rows of 20 tiles, 160x16 pixels) is printed immediately as a new image. The reasons are quite simple : it is easier to code for a lazy guy like me, it avoids printing artifacts due to packets not having the same size (the longer the packets, the darker the print, but the printing streaks becomes more obvious) and it avoids playing too long with the timings which are very stricts in the protocol. A drawback of this protocol is that the images appears a bit "softer" than the one printed directly with a Game Boy. The printer is anyway not famous for its contrast. During the whole protocol, I do not check wether the printer is responding or not. You can uncomment lines in TheFakePrinter.ino to see that the Printer is responding, but who cares as soon as it prints ?! Uncomment this line in printing void() and the printer will speak to you in binary langage. 

    //Serial.print(bit_read);

It says "0x81" (10000001) in the response byte if it is alive. If the SD card is not fast enough, it will add "0x08" (00001000), meaning than data are unprocessed. In this case printing fails, you can buy another SD card. To see what the printer send, uncomment this line in the same void () : 

    //Serial.print(bit_sent);

And you will see what the Arduino send to the printer.

# The pinout

Be careful, the default pinout may vary compared to other projects (to adapt depending on your particular SD shield setting). It is recommanded to use a recent SD card as attempts with old ones fail (no print) on my side (I did not make extensive test I admit). The reading speed on SD must be enough to support the amazing baudrate of Game Boy serial protocol.

![Game Boy Printer to Arduino Uno pinout](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Pinout.PNG)
![My personnal setting](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/My_setting.PNG)

# Have fun with it !!!

![Printing a Game Boy Camera image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example2.PNG)
![Printing a random image](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example.PNG)
![Printing a custom banner](https://github.com/Raphael-Boichot/The-FakePrinter/blob/master/Illustrations/Printing_Example3.PNG)
