/*
  Game boy SD Printer with Arduino, by Raphaël BOICHOT 2020/09/02
  This code takes control of the Game Boy Printer like a real Game Boy will do
  You can add a LED on pin13 to see the access to SD card.
  I adopt a textfile format in command to be consistent with data file on SD card, even if is is not usefull, it eases the reading
*/
#include <SD.h>
// Beware of the CS pin of your own SD shield, here it's 4, may be 10 for example
const int chipSelect = 4;
int semibyte1,semibyte2,bit_sent,bit_read,byte_read,byte_sent,i,j,k,m;
int clk = 2; // clock signal
int TX = 3; // The data signal coming from the Arduino and goind to the printer (Sout on Arduino becomes Sin on the printer)
int RX = 5;// The response bytes coming from printer going to Arduino (Sout from printer becomes Sin on the Arduino)
int pos = 1;
int packet_absolute=0;
int packet_number=0;
int mem_packets=9;
char variable,character1,character2,character,file_char,junk;
// if you modify a command, the checksum bytes must be modified accordingly
char INIT[]="88 33 01 00 00 00 01 00 00 00"; //INT command
char PRNT[]="88 33 02 00 04 00 01 00 E4 80 6B 01 00 00"; //PRINT without feed lines
char INQU[]="88 33 0F 00 00 00 0F 00 00 00"; //INQUIRY command
char ZERO[]="88 33 04 00 00 00 04 00 00 00"; //Null data packet, mandatory for validate DATA packet
char DATA[]="88 33 04 00 80 02"; //DATA packet header, considering 640 bytes length by defaut (80 02), the footer is calculated onboard
word checksum =0;
byte checksum_byte1=0;
byte checksum_byte2=0;

void setup() {

  pinMode(clk, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(RX, INPUT_PULLUP);
  digitalWrite(clk, HIGH);
  digitalWrite(TX, LOW);
  int mode=1; //prints Arduino->Printer communication
  //int mode=2; //prints Printer->Arduino communication
  //int mode=3; //minimal serial output (faster)

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);

  Serial.print("Initializing SD card...");
  // if error message while SD inserted, check the CS pin number
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Did you change the chipSelect pin to match your shield or module ?");
    while (true);
  }
  Serial.println("Arduino SD Game Boy printer by Raphaël BOICHOT 2020 ");
  Serial.println("initialization done, printing starts in 5 seconds");
  delay(5000);
  Serial.println("Init"); 
  sequence(INIT, 10, mode,10);// here we send the INIT command, just one time to initiate protocol

  //////////////////////////////////////////////////////////////////beginning of printing session
  File dataFile2 = SD.open("Hex_data.txt"); // this is the file loaded on the SD card

     Serial.println(' ');
  while (dataFile2.available()) {
   packet_number=packet_number+1;
   packet_absolute=packet_absolute+1;
   Serial.println("New stream of data"); 
   Serial.println("discarding junk data"); 
   character=dataFile2.read(); // we first discard junk data header used to match the DATA packet size with the SD sector size
   while (character!='R') {//Last junk character before data
   character=dataFile2.read();    
   }
   sequence(DATA, 6, mode,-1);// here we send the data packet header
   checksum=0x04+0x80+0x02;// this is the cheksum of the data header part
   
   if (mode==3){
    Serial.print("...640 bytes of DATA sent in double speed mode... ");
   }
   for (int i = 1; i <=640; i++) { //here we start reading data into the file on SD card
   semibyte1=indentification(dataFile2.read());
   semibyte2=indentification(dataFile2.read());
   junk=dataFile2.read(); 
   if (i==640){
    dataFile2.read();// one last character to junk
   }
   byte_sent=semibyte1*16|semibyte2;
   checksum=checksum+byte_sent;// here we calculate the checksum, must be sent LSB first
   printing(semibyte1,semibyte2, mode,-1);// here we send the 640 bytes of data
   }
    ///////////////////////

   checksum_byte1=checksum&0x00FF;// here we extract the checksum bytes
   checksum_byte2=checksum/256;
   semibyte2=checksum_byte1&0x0F;
   semibyte1=checksum_byte1/16;
   printing(semibyte1,semibyte2, mode,-1);// LSB // sends first part of the checksum
   semibyte2=checksum_byte2&0x0F;
   semibyte1=checksum_byte2/16;
   printing(semibyte1,semibyte2, mode,-1);// MSB // sends second part of the checksum
   printing(0,0,mode,-1);// here the two response bytes are finally written
   printing(0,0,mode,1);// here the two response bytes are finally written
   Serial.println(' ');
   Serial.print("Packet #");
   Serial.print(packet_absolute);
   Serial.print(" received");

////////////////////////////////////////////
   if (packet_number==mem_packets){// you can fill the memory buffer with 1 to 9 DATA packets

//// we have to flush now the memory if full with 1 to 9 packets
  Serial.println(' ');
  Serial.println("Empty Packet");
  sequence(ZERO, 10, mode,10);// here we send a mandatory empty packet
    
  Serial.println(' ');
  Serial.println("Printing command");
  sequence(PRNT, 14, mode,14);// here we send the last printing command

 for (int m = 1; m <=mem_packets; m++) {//call iquiry until not busy
    Serial.println(' ');
    Serial.println("Inquiry command");
    delay(1200);// I just used a timer to ease the code but the last inquiry is not busy
    sequence(INQU, 10, mode,10);
 }

 packet_number=0;

   }
///////////////////////////////////////////
  Serial.println(' ');
  }

//// we have to flush now the memory for printing remaining packets
  Serial.println("Empty Packet");
  sequence(ZERO, 10, mode,10);// here we send an mandatory empty packet
  Serial.println(' ');
  Serial.println("Printing command");
  sequence(PRNT, 14, mode,14);// here we send the last printing command

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

int indentification(char variable){ // this function converts characters from file or commands into hexadecimal values
int transmission;
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
return transmission;
}

void printing(int semibyte1, int semibyte2, int mode, int error){// this function prints bytes to the serial
      //byte_sent is the byte concatenated from two hex letter
    byte_sent=semibyte1*16|semibyte2;
    for (j=0; j<=7; j++) {
    bit_sent=bitRead(byte_sent,7-j);
    digitalWrite(clk, LOW);
    digitalWrite(TX, bit_sent);
    delayMicroseconds(32);//double speed mode
    digitalWrite(clk, HIGH);
    bit_read=(digitalRead(RX));
    bitWrite(byte_read,7-j,bit_read);
    delayMicroseconds(32);//double speed mode
    }
    //delayMicroseconds(1490);//critical delay between bytes
    if (mode==1){
      Serial.print(byte_sent,HEX);
      Serial.print(' ');
    }
    if (mode==2){
      Serial.print(byte_read,HEX);
      Serial.print(' ');
    }
    if (error==1){
      error_codes(byte_read);
    }
}

void sequence(char packet[], int len, int mode,int error_byte){
    int pos=0;
    
    for (int i = 1; i <=len; i++) {
    semibyte1=indentification(packet[pos]);
    pos=pos+1;
    semibyte2=indentification(packet[pos]);
    int verbose_mode=-1;
    if (i==error_byte){
      verbose_mode=1;
    }
    
    printing(semibyte1,semibyte2, mode, verbose_mode);
    pos=pos+2;
    }
}

void error_codes(byte error){
  if (bitRead(error,0)==1){Serial.print("/Checksum Error/");}
  if (bitRead(error,1)==1){Serial.print("/Printer busy/");}
  if (bitRead(error,2)==1){Serial.print("/Image data full/");}
  if (bitRead(error,3)==1){Serial.print("/Data in memory/");}
  if (bitRead(error,4)==1){Serial.print("/Packet Error/");}
  if (bitRead(error,5)==1){Serial.print("/Paper jam/");}
  if (bitRead(error,6)==1){Serial.print("/Other error/");}
  if (bitRead(error,7)==1){Serial.print("/Battery low/");}
}
