/*
  Game boy SD Printer with Arduino, by Raphaël BOICHOT 2021/07/30
  This code takes control of the Game Boy Printer like a real Game Boy will do
  You can add a LED on pin13 to see the access to SD card.
  You have two choices to connect the Arduino to an SD card:
  - use a pre-built Arduino SD shield, in this case CS may be 4, 6 or 10 (depends on the source)
  - use a generic SD breakout board and wire like this:
  GND<->GND
  +5V<->+5V (these board have a 3.3V<->5V interface)
  D10<->CS (means you have to change the Chipselect value to 10, see comment line 18)
  D11<->MOSI
  D12<->MISO
  D13<->SCK
  This pinout is mandatory for Arduino Uno, only the CS pin can be moved
*/

#include <SD.h>
// Beware of the CS pin of your own SD shield, here it's 4, may be 6 or 10 for example
const int chipSelect = 4;
int i, j, k, m;
bool bit_sent, bit_read;
byte byte_read, byte_sent, semibyte1, semibyte2;
int clk = 2; // clock signal
int TX = 3; // The data signal coming from the Arduino and goind to the printer (Sout on Arduino becomes Sin on the printer)
int RX = 5;// The response bytes coming from printer going to Arduino (Sout from printer becomes Sin on the Arduino)
//invert TX/RX if it does not work, assuming that everything else is OK
int pos = 1;
int packet_absolute = 0;
int packet_number = 0;
int mem_packets = 9;
char junk;
// if you modify a command, the checksum bytes must be modified accordingly
const byte INIT[] = {0x88, 0x33, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00}; //INT command
//const byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x7F,0x6A,0x01,0x00,0x00}; //PRINT without feed lines, darker
const byte PRNT[] = {0x88, 0x33, 0x02, 0x00, 0x04, 0x00, 0x01, 0x00, 0xE4, 0x40, 0x2B, 0x01, 0x00, 0x00}; //PRINT without feed lines, default
//const byte PRNT[]={0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x00,0xEB,0x00,0x00,0x00}; //PRINT without feed lines, lighter
const byte INQU[] = {0x88, 0x33, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00}; //INQUIRY command
const byte EMPT[] = {0x88, 0x33, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00}; //Empty data packet, mandatory for validate DATA packet
const byte DATA_Header[] = {0x88, 0x33, 0x04, 0x00, 0x80, 0x02}; //DATA packet header, considering 640 bytes length by defaut (80 02), the footer is calculated onboard
byte DATA_SD[649];// data buffer
word checksum = 0;

void setup() {

  pinMode(clk, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(RX, INPUT_PULLUP);
  digitalWrite(clk, HIGH);
  digitalWrite(TX, LOW);
  int mode = 1; //1:prints Arduino->Printer communication  2:prints Printer->Arduino communication  3:minimal serial output (faster)
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);
  Serial.println(' ');
  Serial.println(F("SD initialisation..."));
  // if error message while SD inserted, check the CS pin number
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD initialisation failed !"));
    while (true);
  }
  else
  {
    Serial.println(F("SD initialisation success !"));
  }
  delay(2000);
  Serial.println(' ');
  Serial.println(F("INIT packet sent"));
  sequence(INIT, 9, mode, 9); // here we send the INIT command, just one time to initiate protocol

  //////////////////////////////////////////////////////////////////beginning of printing session
  File dataFile2 = SD.open(F("Hex_data.txt")); // this is the file loaded on the SD card

  while (dataFile2.available()) {
    packet_number = packet_number + 1;
    packet_absolute = packet_absolute + 1;

    Serial.println(' ');
    Serial.print(F("DATA packet#"));
    Serial.print(packet_absolute);
    Serial.println(' ');
    ///////////////////////buffering data packet to avoid SD card speed issues on timing
    checksum = 0;
    for (int i = 0; i <= 5; i++) {
      DATA_SD[i] = DATA_Header[i];
    }
    checksum = 0x04 + 0x80 + 0x02; // this is the cheksum of the data header part
    for (int i = 6; i <= 645; i++) { // here we send the data packet buffered itself
      semibyte1 = indentification(dataFile2.read());
      semibyte2 = indentification(dataFile2.read());
      junk = dataFile2.read();
      byte_sent = semibyte1 * 16 | semibyte2;
      if (i == 645) {
        dataFile2.read();// one last character to junk (line feed)
      }
      checksum = checksum + byte_sent; // Checksum calculation
      DATA_SD[i] = byte_sent; // data buffer
    }
    DATA_SD[646] = checksum & 0x00FF; // here we extract the checksum bytes
    DATA_SD[647] = checksum / 256; // here we extract the checksum bytes
    DATA_SD[648] = 0x00; // response byte 1 (keepalive, not checked)
    DATA_SD[649] = 0x00; // response byte 2 (error messages)
    ///////////////////////end of buffering
    sequence(DATA_SD, 649, mode, 649); //here we send the data packet to the printer

    ////////////////////////////////////////////
    if (packet_number == mem_packets) { // you can fill the memory buffer with 1 to 9 DATA packets

      //// we have to flush now the memory if full with 1 to 9 packets
      Serial.println(' ');
      Serial.println(F("EMPT packet sent"));
      sequence(EMPT, 9, mode, 9); // here we send a mandatory empty packet

      Serial.println(' ');
      Serial.println(F("PRNT packet sent"));
      sequence(PRNT, 13, mode, 13); // here we send the last printing command

      for (int m = 1; m <= mem_packets; m++) { //call iquiry until not busy
        Serial.println(' ');
        Serial.println(F("INQU packet sent"));
        sequence(INQU, 9, mode, 9);
        delay(1200);// I just used a timer to ease the code but the last inquiry is not busy
      }
      packet_number = 0; // initialise after flushing memory
    }
    ///////////////////////////////////////////
  }
  Serial.println(' ');
  Serial.println(F("Now flushing memory"));
  //// we have to flush now the memory for printing remaining packets

  Serial.println(F("EMPT packet sent"));
  sequence(EMPT, 9, mode, 9); // here we send an mandatory empty packet
  Serial.println(' ');
  Serial.println(F("PRNT packet sent"));
  sequence(PRNT, 13, mode, 13); // here we send the last printing command

  packet_number = 0;

  digitalWrite(clk, LOW);
  digitalWrite(TX, LOW);
  Serial.println(' ');
  Serial.println(F("End of transmission, reboot Arduino to restart"));
  dataFile2.close();
}

///////////////////////////////////////////////////////////end of printing section

void loop() {
  // empty, the code is just ran one time
}

int indentification(char variable) { // this function converts characters from file or commands into hexadecimal values
  int transmission;
  if (variable == '0') {
    transmission = 0x0;
  }
  if (variable == '1') {
    transmission = 0x1;
  }
  if (variable == '2') {
    transmission = 0x2;
  }
  if (variable == '3') {
    transmission = 0x3;
  }
  if (variable == '4') {
    transmission = 0x4;
  }
  if (variable == '5') {
    transmission = 0x5;
  }
  if (variable == '6') {
    transmission = 0x6;
  }
  if (variable == '7') {
    transmission = 0x7;
  }
  if (variable == '8') {
    transmission = 0x8;
  }
  if (variable == '9') {
    transmission = 0x9;
  }
  if (variable == 'A') {
    transmission = 0xA;
  }
  if (variable == 'B') {
    transmission = 0xB;
  }
  if (variable == 'C') {
    transmission = 0xC;
  }
  if (variable == 'D') {
    transmission = 0xD;
  }
  if (variable == 'E') {
    transmission = 0xE;
  }
  if (variable == 'F') {
    transmission = 0xF;
  }
  return transmission;
}

void printing(int byte_sent, int mode, int error) { // this function prints bytes to the serial
  //byte_sent is the byte concatenated from two hex letter
  for (j = 0; j <= 7; j++) {
    bit_sent = bitRead(byte_sent, 7 - j);
    digitalWrite(clk, LOW);
    digitalWrite(TX, bit_sent);
    delayMicroseconds(30);//double speed mode
    digitalWrite(clk, HIGH);
    bit_read = (digitalRead(RX));
    bitWrite(byte_read, 7 - j, bit_read);
    delayMicroseconds(30);//double speed mode
  }
  delayMicroseconds(0);//optionnal delay between bytes, may me less than 1490 µs
  if (mode == 1) {
    if (byte_sent <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(byte_sent, HEX);
    Serial.print(' ');
  }
  if (mode == 2) {
    if (byte_read <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(byte_read, HEX);
    Serial.print(' ');
  }
  if (error == 1) {
    Serial.print("//");
    for (m = 0; m <= 7; m++) {
      Serial.print(bitRead(byte_read, 7 - m));
    }
  }
}
void sequence(byte packet[], int len, int mode, int error_byte) {
  for (int i = 0; i <= len; i++) {
    int verbose_mode = -1;
    if (i == error_byte) {
      verbose_mode = 1;
    }
    printing(packet[i], mode, verbose_mode);
  }
}
