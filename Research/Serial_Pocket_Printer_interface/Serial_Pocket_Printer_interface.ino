/*Game boy Printer interface with Arduino, by Raphaël BOICHOT 2023/04/03

this code allows interfacing any program with a Pocket Printer via serial port
One byte must be sent to the printer via serial, then one byte must be read on the serial, and so on. The read/write sequence allows a control of the flow

The general protocol to use this code is like this: 
- send an INIT command (it's shelf life is infinite)
- wait 100 ms
- send DATA packets
- wait 100 ms
- send an empty DATA packets (no payload)
- wait 100 ms
- send a PRINT data packets
- wait 100 ms
- send INQU data packets until print is over or just wait 1200 ms per packet sent
*/

char byte_read;
bool bit_sent, bit_read;
int clk = 2; // clock signal
int TX = 3; // The data signal coming from the Arduino and goind to the printer (Sout on Arduino becomes Sin on the printer)
int RX = 5;// The response bytes coming from printer going to Arduino (Sout from printer becomes Sin on the Arduino)
//invert TX/RX if it does not work, assuming that everything else is OK
void setup() {
  pinMode(clk, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(RX, INPUT_PULLUP);
  digitalWrite(clk, HIGH);
  digitalWrite(TX, LOW);
  Serial.begin(9600);
  Serial.println("Waiting for data");
}

void loop() {
  if (Serial.available() > 0)
  {
    Serial.write(printing(Serial.read()));
  }
}

char printing(char byte_sent) { // this function prints bytes to the serial
  for (int i = 0; i <= 7; i++) {
    bit_sent = bitRead(byte_sent, 7 - i);
    digitalWrite(clk, LOW);
    digitalWrite(TX, bit_sent);
    delayMicroseconds(30);//double speed mode
    digitalWrite(clk, HIGH);
    bit_read = (digitalRead(RX));
    bitWrite(byte_read, 7 - i, bit_read);
    delayMicroseconds(30);//double speed mode
  }
  delayMicroseconds(0);//optionnal delay between bytes, may be less than 1490 µs
//  Serial.println(byte_sent, HEX);
//  Serial.println(byte_read, HEX);
return byte_read;
}
