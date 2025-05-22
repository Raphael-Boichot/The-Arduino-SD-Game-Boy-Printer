/*
  Game Boy Printer minimalistic tester with Arduino, by Raphaël BOICHOT 2025/05/20
  This code takes control of the Game Boy Printer like a real Game Boy will do
  It is meant to easily test packets, timings and protocol quirks
  it embeds all what you need to play with printer
  see https://gbdev.gg8.se/wiki/articles/Gameboy_Printer for protocol detail
*/
#include "payload.h"
bool bit_sent, bit_read;
bool state_printer_busy = 0;
bool state_printer_connected = 0;
byte byte_sent;
int CLOCK_pin = 2;  // clock signal
int TX_pin = 3;     // The data signal coming from the Arduino and going to the printer (Sout on Arduino becomes Sin on the printer)
int RX_pin = 4;     // The response bytes coming from printer going to Arduino (Sout from printer becomes Sin on the Arduino)
//invert TX_pin/RX_pin if it does not work, assuming that everything else is OK

byte palette = 0xE4;    // 0x00 is treated as default (= 0xE4)
byte intensity = 0x40;  //default intensity is 0x40, min is 0x00, max is 0x7F, values between 0x80 and 0xFF are treated as default
byte margin = 0x01;     //high nibble, upper margin, low nibble, lower margin, that simple
byte sheets = 0x01;     //Number of sheets to print (0-255). 0 means line feed only.

// if you modify a command, the checksum bytes must be modified accordingly if it's not a const
const byte INIT[] = { 0x88, 0x33, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };                                //Init command, will never change
byte PRNT[] = { 0x88, 0x33, 0x02, 0x00, 0x04, 0x00, sheets, margin, palette, intensity, 0x00, 0x00, 0x00, 0x00 };  //Print command without feed lines
const byte EMPT[] = { 0x88, 0x33, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 };                                //Empty data packet, mandatory for preparing printing, will never change
byte DATA[650];                                                                                                    //generic data packet
const byte ABOR[] = { 0x88, 0x33, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 };                                //Abort sequence, rarely used by games, will never change
const byte INQU[] = { 0x88, 0x33, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00 };                                //Inquiry command, will never change

void setup() {
  pinMode(CLOCK_pin, OUTPUT);
  pinMode(TX_pin, OUTPUT);
  pinMode(RX_pin, INPUT_PULLUP);
  digitalWrite(CLOCK_pin, HIGH);
  digitalWrite(TX_pin, LOW);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial)
    ;

  ping_the_printer();  //printer initialization
  ////////////////////////printing part per se///////////////////////////
  transmit_data_packet(fourty_tiles_DATA, 640);  //packet formatting
  for (int i = 0; i < 2; i++) {
    transmit_data_packet(twenty_tiles_DATA, 320);  //packet formatting
  }
  for (int i = 0; i < 4; i++) {
    transmit_data_packet(twenty_tiles_DATA, 160);  //packet formatting
  }
  for (int i = 0; i < 40; i++) {
    transmit_data_packet(one_tile_DATA, 16);  //packet formatting
  }
  for (int i = 0; i < 640; i++) {
    transmit_data_packet(one_byte_DATA, 1);  //packet formatting
  }
  ////////////////////////printing part per se///////////////////////////

  digitalWrite(CLOCK_pin, LOW);
  digitalWrite(TX_pin, LOW);
  Serial.println();
  Serial.println(F("End of transmission, reboot Arduino to restart"));
}

void loop() {
  // empty, the code is just ran one time at the moment
}

void printing_loop() {
  state_printer_busy = 1;       //to enter the loop
  delay(200);                   //printer is not immediately busy
  while (state_printer_busy) {  //call iquiry until not busy
    Serial.println();
    Serial.print(F("INQU packet sent -->"));
    state_printer_busy = 0;
    send_printer_packet(INQU, 10);
    delay(200);
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

void printing(int byte_sent, int mode, int error_check, int connection_check) {  // this function prints bytes to the serial
  byte byte_read;
  for (int j = 0; j <= 7; j++) {
    bit_sent = bitRead(byte_sent, 7 - j);
    digitalWrite(CLOCK_pin, LOW);
    digitalWrite(TX_pin, bit_sent);
    delayMicroseconds(30);  //double speed mode
    digitalWrite(CLOCK_pin, HIGH);
    bit_read = (digitalRead(RX_pin));
    bitWrite(byte_read, 7 - j, bit_read);
    delayMicroseconds(30);  //double speed mode
  }
  delayMicroseconds(0);  //optionnal delay between bytes, may me less than 1490 µs
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
  packet[checksum_pos] = checksum & 0x00FF;  // low byte
  packet[checksum_pos + 1] = checksum >> 8;  // high byte
}

//checksum is always from the third to the last-4 bytes
void update_size(byte* packet, int size_pos, word data_size) {
  packet[size_pos] = data_size & 0x00FF;  // low byte
  packet[size_pos + 1] = data_size >> 8;  // high byte
}

void transmit_data_packet(byte* packet, word data_size) {
  int size_start = 4;
  int checksum_start = 2;
  int checksum_end = data_size + 5;        // Reserve 2 bytes for checksum
  int total_packet_size = data_size + 10;  // Safety margin for checksum + header/trailer if needed
  Serial.println();
  Serial.print(F("Updating DATA packet checksum for size "));
  Serial.print(data_size);
  update_size(packet, size_start, data_size);                               //size first as it is into the checksum
  update_checksum(packet, checksum_start, checksum_end - 1, checksum_end);  // update checksum at last so
  Serial.println();
  Serial.print(F("DATA packet sent --> "));
  send_printer_packet(packet, total_packet_size);  // Send complete packet
}

void finalize_and_print() {
  Serial.println();
  Serial.print(F("EMPT packet sent -->"));
  send_printer_packet(EMPT, 10);  // here we send a mandatory empty packet with 0 payload
  Serial.println();
  Serial.print(F("PRNT packet sent -->"));
  update_checksum(PRNT, 2, 9, 10);
  send_printer_packet(PRNT, 14);  // here we send the last printing command
  printing_loop();                // flux control
}

void ping_the_printer() {
  Serial.println();
  Serial.print(F("Trying to ping the printer"));
  while (!(state_printer_connected)) {
    delay(500);
    Serial.println();
    Serial.print(F("INIT packet sent -->"));
    send_printer_packet(INIT, 10);  // here we send the INIT command until we get 0x81 at byte 9, printer connected
    if (!(state_printer_connected)) {
      Serial.print(F(" / Printer not responding"));
    }
  }
  Serial.print(F(" / Printer connected !"));
}