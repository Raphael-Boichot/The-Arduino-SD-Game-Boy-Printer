/*
  Game Boy Printer SD printer with Arduino, by Raphaël BOICHOT 2025/05/20
  This code takes control of the Game Boy Printer like a real Game Boy will do
  It is meant to easily test packets, timings and protocol quirks
  it embeds all what you need to play with printer
  see https://gbdev.gg8.se/wiki/articles/Gameboy_Printer for protocol detail
  You can add a LED on pin13 to see the access to SD card.
  You have two choices to connect the Arduino to an SD card:
  - use a pre-built Arduino SD shield, in this case CS may be 4, 6 or 10 (depends on the source)
  - use a generic SD breakout board and wire like this:
  GND<->GND
  +5V<->+5V (these board have a 3.3V<->5V interface)
  D10<->CS 
  D11<->MOSI
  D12<->MISO
  D13<->SCK
  This pinout is mandatory for Arduino Uno, only the CS pin can be moved
*/
#include <SD.h>
#define CLOCK_DELAY 30           // delay between bits µs (30 is double speed mode)
#define OPTIONAL_DELAY 0         // delay between bytes µs (cannot exceed 1490 µs)
#define PRINTING_LOOP_DELAY 250  // delay between packets ms, when useful
const int chipSelect = 10;
bool bit_sent, bit_read;
bool state_printer_busy = 0;
bool state_printer_connected = 0;
int packet_number = 0;
byte byte_sent, byte_junk, upper_nibble, lower_nibble;
int CLOCK_pin = 2;  //clock signal
int TX_pin = 3;     //The data signal coming from the Arduino and going to the printer (Sout on Arduino becomes Sin on the printer)
int RX_pin = 4;     //The response bytes coming from printer going to Arduino (Sout from printer becomes Sin on the Arduino)
//invert TX_pin/RX_pin if it does not work, assuming that everything else is OK

// printing parameters
byte palette = 0xE4;    //0x00 is treated as default (= 0xE4)
byte intensity = 0x40;  //default intensity is 0x40, min is 0x00, max is 0x7F, values between 0x80 and 0xFF are treated as default
byte margin = 0x00;     //high nibble, upper margin, low nibble, lower margin, that simple
byte sheets = 0x01;     //Number of sheets to print (0-255). 0 means line feed only.

// if you modify a command, the checksum bytes must be modified accordingly if it's not a const
const byte INIT[] = { 0x88, 0x33, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };                                //Init command, will never change
byte PRNT[] = { 0x88, 0x33, 0x02, 0x00, 0x04, 0x00, sheets, margin, palette, intensity, 0x00, 0x00, 0x00, 0x00 };  //Print command without feed lines
const byte EMPT[] = { 0x88, 0x33, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 };                                //Empty data packet, mandatory for preparing printing, will never change
const byte ABOR[] = { 0x88, 0x33, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 };                                //Abort sequence, rarely used by games, will never change
const byte INQU[] = { 0x88, 0x33, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00 };                                //Inquiry command, will never change

//note: arbitrary data packet length can be used as long as the total data in printer memory are multiple of 640 bytes before printing
//this means you can send 2 consecutives 320 bytes packets instead of one 640 bytes, it is the same
//not respecting this rule will force the printer to make a memory overflow and print garbage (but it prints)
//a 640 bytes packet of data in Game Boy Tile Format for debugging (or more)
byte DATA[] = { 0x88, 0x33, 0x04, 0x00, 0x80, 0x02,                                                              //header
                0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8,  //payload, first tile
                0x18, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x18, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x18, 0x00, 0x3C,
                0xC0, 0xC0, 0x06, 0x00, 0x06, 0x00, 0xC0, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xC0, 0x00, 0x00, 0x06,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x01, 0x01, 0x03, 0x02, 0x07, 0x04,
                0x0E, 0x0F, 0x1F, 0x31, 0x3E, 0x60, 0x7D, 0xC0, 0xF9, 0x84, 0xF3, 0x0F, 0xEF, 0x1F, 0xF8, 0x34,
                0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF0, 0xF0, 0xF8, 0xF8, 0xF8, 0xF8, 0x80, 0x80,
                0x00, 0x05, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
                0x00, 0x55, 0x00, 0x00, 0xFF, 0xFF, 0xFE, 0x9C, 0xBE, 0xDC, 0xFE, 0x9C, 0xBE, 0xDC, 0xFE, 0x80,
                0x00, 0x55, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0xCF, 0xCF, 0xDF, 0xD8, 0xDF, 0xD3, 0xD7, 0xDB,
                0x00, 0x55, 0x00, 0x00, 0x07, 0x07, 0x07, 0x04, 0xE7, 0xE6, 0xF7, 0x76, 0xB3, 0x32, 0xB3, 0x32,
                0x00, 0x55, 0x00, 0x00, 0xE7, 0xE7, 0x67, 0x64, 0x67, 0x66, 0x67, 0x66, 0x63, 0x62, 0x63, 0x62,
                0x00, 0x55, 0x00, 0x00, 0xE0, 0xE0, 0x60, 0x60, 0x63, 0x63, 0x67, 0x66, 0x67, 0x64, 0x65, 0x66,
                0x00, 0x55, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0xF9, 0xF9, 0xFD, 0x1D, 0xED, 0xCD, 0xED, 0xCD,
                0x00, 0x55, 0x00, 0x00, 0xF0, 0xF0, 0xB0, 0x30, 0xB0, 0x30, 0xB0, 0x30, 0xB0, 0x30, 0xB0, 0x30,
                0x00, 0x00, 0x01, 0x41, 0x02, 0x02, 0x02, 0x42, 0x02, 0x02, 0x02, 0x42, 0x02, 0x02, 0x02, 0x42,
                0xFE, 0xFE, 0x83, 0xFF, 0xFE, 0x82, 0x82, 0xFE, 0xFE, 0xFE, 0x82, 0xFE, 0xFE, 0xFE, 0x82, 0xFE,
                0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0xE2, 0x80, 0xE0, 0x80, 0xE0, 0x80, 0xE0, 0x80,
                0x03, 0x00, 0x00, 0x60, 0x00, 0x60, 0x00, 0x03, 0x00, 0x07, 0x00, 0x07, 0x00, 0x03, 0x60, 0x60,
                0x00, 0x18, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x18, 0x00, 0x80, 0x00, 0x80, 0x18, 0x18, 0x3C, 0x3C,
                0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B,
                0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8, 0xC3, 0xD8,
                0x00, 0x3C, 0x00, 0x18, 0x00, 0x01, 0x00, 0x01, 0x18, 0x18, 0x3C, 0x3C, 0x3C, 0x3C, 0x18, 0x18,
                0x00, 0x06, 0x00, 0xC0, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xC0, 0x06, 0x06, 0x06, 0x06, 0xC0, 0xC0,
                0x07, 0x04, 0x05, 0x07, 0x07, 0x07, 0x43, 0x03, 0x01, 0x03, 0x01, 0x01, 0x03, 0x03, 0x00, 0x00,
                0xFD, 0xE1, 0xFD, 0xF1, 0xCF, 0xF0, 0xFD, 0xCE, 0xFF, 0xAF, 0xBF, 0xC3, 0x7F, 0xE0, 0x1F, 0x3F,
                0xE0, 0xE0, 0x70, 0x98, 0xF8, 0x08, 0xF0, 0x18, 0xE0, 0xF0, 0x80, 0xC5, 0x80, 0x80, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x15,
                0xBE, 0xDC, 0xFE, 0x9C, 0xBE, 0xDC, 0xFE, 0x9C, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x55,
                0xDF, 0xD0, 0xD7, 0xDB, 0xDF, 0xD3, 0xDF, 0xD8, 0xDF, 0xDF, 0xCF, 0xCF, 0x00, 0x00, 0x00, 0x55,
                0xB3, 0x32, 0xF3, 0xF2, 0x33, 0x32, 0x73, 0x72, 0xF3, 0xF3, 0xE3, 0xE3, 0x00, 0x00, 0x00, 0x55,
                0x63, 0x62, 0x63, 0x62, 0x63, 0x62, 0x63, 0x62, 0xE3, 0xE3, 0xE3, 0xE3, 0x00, 0x00, 0x00, 0x55,
                0x67, 0x64, 0x65, 0x66, 0x67, 0x64, 0x67, 0x66, 0xE7, 0xE7, 0xE3, 0xE3, 0x00, 0x00, 0x00, 0x55,
                0xED, 0xCD, 0xED, 0xCD, 0xED, 0xCD, 0xFD, 0x1D, 0xFD, 0xFD, 0xF9, 0xF9, 0x00, 0x00, 0x00, 0x55,
                0xB0, 0x30, 0xB0, 0x30, 0xF0, 0xF0, 0xB0, 0x30, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x55,
                0x02, 0x02, 0x02, 0x42, 0x02, 0x02, 0x02, 0x42, 0x02, 0x02, 0x02, 0x42, 0x03, 0x03, 0x00, 0x40,
                0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x06, 0xC0, 0x06, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
                0xE0, 0x80, 0xE0, 0x80, 0xE0, 0x80, 0xE2, 0x80, 0xE0, 0x80, 0xE0, 0x80, 0xE0, 0x80, 0xE0, 0x00,
                0x60, 0x60, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x03, 0x03, 0x60, 0x00, 0x60, 0x00, 0x03, 0x00,
                0x3C, 0x3C, 0x18, 0x18, 0x80, 0x80, 0x80, 0x80, 0x18, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x18, 0x00,
                0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B, 0xC3, 0x1B,  //payload, last
                0x00, 0x00, 0x00, 0x00 };                                                                        //footer

void setup() {
  pinMode(CLOCK_pin, OUTPUT);
  pinMode(TX_pin, OUTPUT);
  pinMode(RX_pin, INPUT_PULLUP);
  digitalWrite(CLOCK_pin, HIGH);
  digitalWrite(TX_pin, LOW);
  Serial.begin(115200);
  while (!Serial)  //Open serial communications and wait for port to open:
    ;
  Serial.println(' ');
  Serial.println(F("SD initialisation..."));
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD initialisation failed !"));
    while (true)
      ;
  } else {
    Serial.println(F("SD initialisation success !"));
  }
  ping_the_printer();  //printer initialization

  ///////////////////////main "loop"///////////////////////////////////////////////////
  File dataFile2 = SD.open(F("Hex_data.txt"));  //this is the file loaded on the SD card
  while (dataFile2.available()) {
    packet_number = packet_number + 1;
    for (int i = 6; i <= 645; i++) {  //filling data packet with data from SD card
      upper_nibble = Ascii_to_nibble(dataFile2.read());
      lower_nibble = Ascii_to_nibble(dataFile2.read());
      byte_junk = dataFile2.read();
      DATA[i] = upper_nibble * 16 | lower_nibble;
      if (i == 645) {
        dataFile2.read();  //one last character to drop (line feed)
      }
    }
    transmit_data_packets_640();  //fixes packet checksum and bitbangs the printer
    if (packet_number == 9) {     //you can fill the memory buffer with 1 to 9 DATA packets
      finalize_and_print();       //sends empty payload and print command
      packet_number = 0;          //loop on itself
    }
  }
  ///////////////////////main "loop"///////////////////////////////////////////////////
  finalize_and_print();  //flushing spooler with remaining packets in printer memory
  digitalWrite(CLOCK_pin, LOW);
  digitalWrite(TX_pin, LOW);
  Serial.println();
  Serial.println(F("End of transmission, reboot Arduino to restart"));
  dataFile2.close();
}

void loop() {
  // empty, the code is just ran one time at the moment
}

void printing_loop() {
  state_printer_busy = 1;       //to enter the loop
  delay(PRINTING_LOOP_DELAY);   //printer is not immediately busy
  while (state_printer_busy) {  //call iquiry until not busy
    Serial.println();
    Serial.print(F("INQU packet sent -->"));
    state_printer_busy = 0;
    send_printer_packet(INQU, 10);
    delay(PRINTING_LOOP_DELAY);
  }
  Serial.println();
  Serial.print(F("Printer not busy !"));
}

void send_printer_packet(byte packet[], int sequence_length) {
  for (int i = 0; i <= sequence_length - 1; i++) {
    int error_check = (i == sequence_length - 1) ? 1 : -1;
    int connection_check = (i == sequence_length - 2) ? 1 : -1;
    int mode = ((i == sequence_length - 1) || (i == sequence_length - 2)) ? 2 : 1;
    printing(packet[i], mode, error_check, connection_check);
  }
}

void printing(int byte_sent, int mode, int error_check, int connection_check) {  //this function prints bytes to the serial
  byte byte_read;
  for (int j = 0; j <= 7; j++) {
    bit_sent = bitRead(byte_sent, 7 - j);
    digitalWrite(CLOCK_pin, LOW);
    digitalWrite(TX_pin, bit_sent);
    delayMicroseconds(CLOCK_DELAY);
    digitalWrite(CLOCK_pin, HIGH);
    bit_read = (digitalRead(RX_pin));
    bitWrite(byte_read, 7 - j, bit_read);
    delayMicroseconds(CLOCK_DELAY);
  }
  delayMicroseconds(OPTIONAL_DELAY);
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

  if (connection_check == 1) {
    state_printer_connected = 0;
    if (byte_read == 0x81) {
      state_printer_connected = 1;
    };
  }

  if (error_check == 1) {
    Serial.print("--> ");
    for (int m = 0; m <= 7; m++) { Serial.print(bitRead(byte_read, 7 - m)); }
    if (bitRead(byte_read, 0)) { Serial.print(F(" / Checksum error")); }
    state_printer_busy = 0;
    if (bitRead(byte_read, 1)) {
      state_printer_busy = 1;
      Serial.print(F(" / Printer busy"));
    }
    if (bitRead(byte_read, 2)) { Serial.print(F(" / Image data full")); }
    if (bitRead(byte_read, 3)) { Serial.print(F(" / Unprocessed data")); }
    if (bitRead(byte_read, 4)) { Serial.print(F(" / Packet error")); }
    if (bitRead(byte_read, 5)) { Serial.print(F(" / Paper jam")); }
    if (bitRead(byte_read, 6)) { Serial.print(F(" / Other error")); }
    if (bitRead(byte_read, 7)) { Serial.print(F(" / Low battery")); }
  }
}

//checksum is always from the third to the last-4 bytes
void update_checksum(byte* packet, int start_index, int end_index, int checksum_pos) {
  word checksum = 0;
  for (int i = start_index; i <= end_index; i++) {
    checksum += packet[i];
  }
  packet[checksum_pos] = checksum & 0x00FF;  //low byte
  packet[checksum_pos + 1] = checksum >> 8;  //high byte
}

void transmit_data_packets_640() {
  Serial.println();
  Serial.print(F("Updating DATA packet checksum"));
  update_checksum(DATA, 2, 645, 646);
  Serial.println();
  Serial.print(F("DATA packet sent -->"));
  send_printer_packet(DATA, 650);  //here we send the data packet to the printer
}

void finalize_and_print() {
  Serial.println();
  Serial.print(F("EMPT packet sent -->"));
  send_printer_packet(EMPT, 10);  //here we send a mandatory empty packet with 0 payload
  Serial.println();
  Serial.print(F("PRNT packet sent -->"));
  update_checksum(PRNT, 2, 9, 10);
  send_printer_packet(PRNT, 14);  //here we send the last printing command
  printing_loop();                //flux control
}

void ping_the_printer() {
  Serial.println();
  Serial.print(F("Trying to ping the printer"));
  while (!(state_printer_connected)) {
    delay(PRINTING_LOOP_DELAY);
    Serial.println();
    Serial.print(F("INIT packet sent -->"));
    send_printer_packet(INIT, 10);  //here we send the INIT command until we get 0x81 at byte 9, printer connected
    if (!(state_printer_connected)) {
      Serial.print(F(" / Printer not responding"));
    }
  }
  Serial.print(F(" / Printer connected !"));
}

int Ascii_to_nibble(char c) {  //this function converts characters from file or commands into hexadecimal values
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;  //fallback for unexpected chars
}