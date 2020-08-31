# The-FakePrinter
Print everything you want with your Game Boy Printer !

This project provide an easy solution to hack the Game Boy Printer. You just need an Arduino and SD shield plus some soldering. The code is divided into two parts : a converter ran with Octave/Matlab to transform any image that fits the format of a GameBoy printed image (160 pixels width, multiple of 16 pixel height, 4 shades of gray) into a tile format.

Details of the image to tile transformation are exposed here for example :
https://blog.flozz.fr/2018/11/19/developpement-gameboy-5-creer-des-tilesets/

This Game Boy tile format made under text file by the Octave converter must be uploaded on SD card, and is then interpreted as a Game Boy Printer protocol by the Arduino code. Example of game bor Printer protocol can be found here : 
https://gbdev.gg8.se/wiki/articles/Gameboy_Printer

The printing is automatic once the Arduino is powered, connected to the Game Boy Printer. Rebooting the Arduino causes another print.
You can also print images from Game Boy Camera or images extracted from games with this code :
https://github.com/mofosyne/arduino-gameboy-printer-emulator

Be careful, the default pinout may vary (to adapt depending on your particular setting).

Have fun with it !!!

