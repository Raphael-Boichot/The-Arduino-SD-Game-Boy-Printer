/*
  SD card file dump

  This example shows how to read a file from the SD card using the
  SD library and send it over the serial port.
  Pin numbers reflect the default SPI pins for Uno and Nano models.

  The circuit:
   SD card attached to SPI bus as follows:
 ** SDO - pin 11
 ** SDI - pin 12
 ** CLK - pin 13
 ** CS - depends on your SD card shield or module.
        Pin 10 used here for consistency with other Arduino examples
   (for MKRZero SD: SDCARD_SS_PIN)

  created  22 December 2010
  by Limor Fried
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.
*/
#include <SD.h>

const int chipSelect = 4;
int semibyte1,semibyte2,bit_sent,bit_read,byte_read,byte_sent,i,j,k,transmission;
int clk = 2; 
int TX = 3;
int RX = 5;//4 is reserved for CS from SD shield
int pos = 1;
char variable,character1,character2,character,file_char,junk;
char INIT[]="88 33 01 00 00 00 01 00 00 00";
//char PRNT[]="88 33 02 00 04 00 01 13 E4 7F 7D 01 00 00"; //with feed lines
char PRNT[]="88 33 02 00 04 00 01 00 E4 80 6B 01 00 00"; //without feed lines
//char PRNT[]="88 33 02 00 04 00 01 00 E4 FF EA 01 00 00"; //without feed lines, the darkest possible
char INQU[]="88 33 0F 00 00 00 0F 00 00 00";
char DATA[]="88 33 04 00 00 00 04 00 00 00";
char EMPT[]="00 00 00 00 00 00 00 00 00 00";
char TEST[]="88 33 04 00 80 02";
char buff[3];
word checksum =0;
byte checksum_byte1=0;
byte checksum_byte2=0;

void setup() {

   pinMode(clk, OUTPUT);
   pinMode(TX, OUTPUT);
   pinMode(RX, INPUT_PULLUP);
  digitalWrite(clk, HIGH);
  digitalWrite(TX, LOW);
  delay(2000);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    while (true);
  }

  Serial.println("initialization done.");

//  // open the file. note that only one file can be open at a time,
//  // so you have to close this one before opening another.
//  File dataFile = SD.open("Hex_data.txt");
//
//  // if the file is available, write to it:
//  if (dataFile) {
//    while (dataFile.available()) {
//      Serial.write(dataFile.read());
//    }
//    dataFile.close();
//  }
//  // if the file isn't open, pop up an error:
//  else {
//    Serial.println("error opening GBCam.txt");
//  }

  //////////////////////////////////////////////////////////////////beginning of printing session
  File dataFile2 = SD.open("Hex_data.txt");
  while (dataFile2.available()) {
  Serial.println("Init");
    pos=0;
    for (int i = 1; i <=10; i++) {
    character=INIT[pos];
    Serial.print(character);
    indentification();
    semibyte1=transmission;
    pos=pos+1;
    character=INIT[pos];
    indentification();
    Serial.print(character);
    semibyte2=transmission;
    byte_sent=semibyte1*16|semibyte2;
    printing();
    pos=pos+2;
    }

  Serial.println(' ');
  Serial.println("New stream of data");
  Serial.println(' ');
   delay(0);
   pos=0;
    for (int i = 1; i <=6; i++) {
    character=TEST[pos];
    Serial.print(character);
    indentification();
    semibyte1=transmission;
    pos=pos+1;
    character=TEST[pos];
    Serial.print(character);
    indentification();
    semibyte2=transmission;
    byte_sent=semibyte1*16|semibyte2;
    printing();
    pos=pos+2;
    }
    checksum=130+4;
    ////////////////////here we read into the file
    for (int i = 1; i <=640; i++) {
    character=dataFile2.read();    
    Serial.print(character);
    indentification();
    semibyte1=transmission;
    character=dataFile2.read(); 
    Serial.print(character);
    indentification();
    semibyte2=transmission;
    junk=dataFile2.read(); 
    //Serial.print(junk);
    if (i==640){
    dataFile2.read();
     }
    //semibyte1=0xF;
    //semibyte2=0xF;
    byte_sent=semibyte1*16|semibyte2;
    checksum=checksum+byte_sent;
    //delay(100);
    printing();
    }
    ///////////////////////

checksum_byte1=checksum&0x00FF;
checksum_byte2=checksum/256;

//Serial.println(' ');
//Serial.println(checksum,HEX);
//Serial.println(checksum_byte1,HEX);
//Serial.println(checksum_byte2,HEX);
//Serial.println(' ');

    byte_sent=checksum_byte1;
    printing();

//semibyte1=0x7;
//semibyte2=0xE;
//byte_sent=semibyte1*16|semibyte2;
    byte_sent=checksum_byte2;
    printing();
 
    for (int i = 1; i <=2; i++) {
    semibyte1=0;
    semibyte2=0;
    byte_sent=semibyte1*16|semibyte2;
    printing();
    }


  Serial.println(' ');
  Serial.println("Empty Packet");
  Serial.println(' ');  
  delay(0);
  pos=0;
    for (int i = 1; i <=10; i++) {
    character=DATA[pos];
    Serial.print(character);
    indentification();
    semibyte1=transmission;
    pos=pos+1;
    character=DATA[pos];
    Serial.print(character);
    indentification();
    semibyte2=transmission;
    byte_sent=semibyte1*16|semibyte2;
    printing();
    pos=pos+2;
    }
  Serial.println(' ');
  Serial.println("Printing command");
  Serial.println(' ');  
  delay(0);
  pos=0;
    for (int i = 1; i <=14; i++) {
    character=PRNT[pos];
    Serial.print(character);
    indentification();
    semibyte1=transmission;
    pos=pos+1;
    character=PRNT[pos];
    Serial.print(character);
    indentification();
    semibyte2=transmission;
    byte_sent=semibyte1*16|semibyte2;
    printing();
    pos=pos+2;
    }
  Serial.println(' ');
  Serial.println(' ');
 delay(2000);
  }
dataFile2.close();
  digitalWrite(clk, LOW);
  digitalWrite(TX, LOW);
  Serial.print("End of transmission");

}

///////////////////////////////////////////////////////////end of printing section


void loop() {

  }

 

  


void indentification(){
 variable=character;
if (variable=='0'){transmission=0x0;}
if (variable=='1'){transmission=0x1;}
if (variable=='2'){transmission=0x2;}
if (variable=='3'){transmission=0x3;}
if (variable=='4'){transmission=0x4;}
if (variable=='5'){transmission=0x5;}
if (variable=='6'){transmission=0x6;}
if (variable=='7'){transmission=0x7;}
if (variable=='8'){transmission=0x8;}
if (variable=='9'){transmission=0x9;}
if (variable=='A'){transmission=0xA;}
if (variable=='B'){transmission=0xB;}
if (variable=='C'){transmission=0xC;}
if (variable=='D'){transmission=0xD;}
if (variable=='E'){transmission=0xE;}
if (variable=='F'){transmission=0xF;}
}

void printing(){
      //Chain is the byte concatenated from two hex letter
    for (j=0; j<=7; j++) {
    bit_sent=bitRead(byte_sent,7-j);
    //Serial.print(bit_sent);
    digitalWrite(clk, LOW);
    digitalWrite(TX, bit_sent);
    delayMicroseconds(63);
    digitalWrite(clk, HIGH);
    bit_read=(digitalRead(RX));
    //Serial.print(bit_read);
    bitWrite(byte_read,7-j,bit_read);
    delayMicroseconds(63);
    }
    delayMicroseconds(123);
    //Serial.print(' ');
    //digitalWrite(TX, LOW);
}
