/*
  Game boy SD Printer with Arduino, by Raphaël BOICHOT 2020/09/02
  This code takes control of the Game Boy Printer like a real Game Boy will do
  You can add a LED on pin13 to see the access to SD card.
*/
#include <SD.h>
// Beware of the CS pin of your own SD shield, here it's 4, may be 10 for example
const int chipSelect = 4;
int semibyte1,semibyte2,bit_sent,bit_read,byte_read,byte_sent,i,j,k,m,transmission;
int clk = 2; // clock signal
int TX = 3; // The data signal coming from the Arduino and goind to the printer (Sout on Arduino becomes Sin on the printer)
int RX = 5;// The response bytes coming from printer going to Arduino (Sout from printer becomes Sin on the Arduino)
int pos = 1;
int inquiry=0;
int packet_absolute=0;
int packet_number=0;
int mem_packets=9;
char variable,character1,character2,character,file_char,junk;
// if you modify a command, the checksum bytes must be modified accordingly
char INIT[]="88 33 01 00 00 00 01 00 00 00"; //INT command
//char PRNT[]="88 33 02 00 04 00 01 13 E4 7F 7D 01 00 00"; //PRINT with feed lines, not used here
char PRNT[]="88 33 02 00 04 00 01 00 E4 80 6B 01 00 00"; //PRINT without feed lines
char INQU[]="88 33 0F 00 00 00 0F 00 00 00"; //INQUIRY command
char DATA[]="88 33 04 00 00 00 04 00 00 00"; //Null data packet, mandatory for validate DATA packet
//char EMPT[]="00 00 00 00 00 00 00 00 00 00"; //Null packet, NOT USED HERE
char TEST[]="88 33 04 00 80 02"; //DATA packet header, considerinf 640 bytes length by defaut (80 02)
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
  // if error message while SD inserted, check the CS pin number
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    while (true);
  }

  Serial.println("initialization done.");

    Serial.println("Init"); // here we send the INT command
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
    Serial.print(' ');
    }

  //////////////////////////////////////////////////////////////////beginning of printing session
  File dataFile2 = SD.open("Hex_data.txt"); // this is the file loaded on the SD card

     Serial.println(' ');
  while (dataFile2.available()) {
   packet_number=packet_number+1;
   packet_absolute=packet_absolute+1;
   Serial.println("New stream of data"); // here we send the data packet
    Serial.println("discarding junk data"); // we first discard dumb data used to match the DATA packet size with the SD sector size
    character=dataFile2.read();    
    while (character!='R') {
    character=dataFile2.read();    
    }
    pos=0;
    inquiry=0;
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
    Serial.print(' ');
    }
    checksum=130+4;
    ////////////////////here we start reading into the file on SD card
    //Serial.print("...640 bytes of DATA... ");
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
    Serial.print(junk);
    if (i==640){
    dataFile2.read();
     }
    byte_sent=semibyte1*16|semibyte2;
    checksum=checksum+byte_sent;
    printing();
    //Serial.print(' ');
    }
    ///////////////////////

checksum_byte1=checksum&0x00FF;// here we calculate the checksum, must be sent LSB first
checksum_byte2=checksum/256;

    byte_sent=checksum_byte1;// LSB
    Serial.print(byte_sent,HEX);
    Serial.print(' ');
    printing();
    byte_sent=checksum_byte2;// MSB
    Serial.print(byte_sent,HEX);
    Serial.print(' ');
    printing();
 
    for (int i = 1; i <=2; i++) {// here the two response bytes are written
    semibyte1=0;
    semibyte2=0;
    byte_sent=semibyte1*16|semibyte2;
    Serial.print(byte_sent);
    printing();
    Serial.print(' ');
    }
inquiry=0;
Serial.println(' ');
Serial.print("Packet #");
Serial.print(packet_absolute);


////////////////////////////////////////////
    if (packet_number==mem_packets){// you can fill the memory buffer with 1 to 9 DATA packets

//// we have to flush now the memory 
  Serial.println(' ');
  Serial.println("Empty Packet");// here we send a mandatory empty packet
  pos=0;
  inquiry=0;
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
    Serial.print(' ');
    }
inquiry=0;
    
Serial.println(' ');
Serial.println("Printing command");// here we send the last printing command
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
    Serial.print(' ');
     }

 for (int m = 1; m <=mem_packets; m++) {//call iquiry until not busy
    Serial.println(' ');
Serial.println("Inquiry command");// I just used a timer to ease the code
    delay(1200);
    pos=0;
    inquiry=1;
    for (int i = 1; i <=10; i++) {
    character=INQU[pos];
    //Serial.print(character);
    indentification();
    semibyte1=transmission;
    pos=pos+1;
    character=INQU[pos];
    //Serial.print(character);
    indentification();
    semibyte2=transmission;
    byte_sent=semibyte1*16|semibyte2;
    printing();
    pos=pos+2;
    //Serial.print(' ');
    }
    inquiry=0;
 }

 packet_number=0;

   }
///////////////////////////////////////////
  Serial.println(' ');
  }

//// we have to flush now the memory for printing remaining packets
  Serial.println("Empty Packet");// here we send an mandatory empty packet
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
    Serial.print(' ');
    }
Serial.println(' ');
Serial.println("Printing command");// here we send the last printing command
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
    Serial.print(' ');
     }

 packet_number=0;
 
  digitalWrite(clk, LOW);
  digitalWrite(TX, LOW);
  Serial.println(' ');
  Serial.print("End of transmission, reboot to print again or load another text file");
  dataFile2.close();
}

///////////////////////////////////////////////////////////end of printing section

void loop() {
// empty, the code is just ran one time
  }

void indentification(){ // this function converts characters into hexadecimal values
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

void printing(){// this function prints bytes
      //Chain is the byte concatenated from two hex letter
    for (j=0; j<=7; j++) {
    bit_sent=bitRead(byte_sent,7-j);
    //Serial.print(bit_sent); // uncomment to see what Arduino send (in binary)
    digitalWrite(clk, LOW);
    digitalWrite(TX, bit_sent);
    delayMicroseconds(63);
    digitalWrite(clk, HIGH);
    bit_read=(digitalRead(RX));
    //Serial.print(bit_read); // uncomment to see what the printer send (in binary)
if (inquiry==1){
  Serial.print(bit_read);
}
    
    
    bitWrite(byte_read,7-j,bit_read);
    delayMicroseconds(63);
    }
    delayMicroseconds(127);
    //Serial.print(' ');
    //digitalWrite(TX, LOW);
}
