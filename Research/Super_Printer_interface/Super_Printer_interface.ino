#define PRINT_PAYLOAD_SIZE 3
#define DATA_PAYLOAD_SIZE 640
#define DATA_TOTAL_SIZE 650
#define DATA_PAYLOAD_OFFSET 6

uint8_t printBuffer[PRINT_PAYLOAD_SIZE];
uint8_t dataBuffer[DATA_TOTAL_SIZE];

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  // Wait for serial
}

void loop() {
  if (Serial.available()) {
    char packetType = Serial.read();

    if (packetType == 'P') {
      if (receiveAndStorePayload(printBuffer, PRINT_PAYLOAD_SIZE)) {
        echoPacket('P', printBuffer, PRINT_PAYLOAD_SIZE);
      } else {
        flushSerialInput();
      }
    } else if (packetType == 'D') {
      if (receiveAndStorePayload(dataBuffer + DATA_PAYLOAD_OFFSET, DATA_PAYLOAD_SIZE)) {
        echoPacket('D', dataBuffer + DATA_PAYLOAD_OFFSET, DATA_PAYLOAD_SIZE);
      } else {
        flushSerialInput();
      }
    }
  }
}

bool receiveAndStorePayload(uint8_t* buffer, size_t length) {
  size_t received = 0;
  unsigned long timeout = millis() + 70;

  while (received < length && millis() < timeout) {
    if (Serial.available()) {
      buffer[received++] = Serial.read();
    }
  }

  // Wait for CR terminator
  while (millis() < timeout) {
    if (Serial.available()) {
      char terminator = Serial.read();
      return terminator == '\r';
    }
  }
  return false;
}

void echoPacket(char type, uint8_t* buffer, size_t length) {
  Serial.write(type);
  Serial.write(buffer, length);
  Serial.write('\r');  // Echo CR
}

void flushSerialInput() {
  while (Serial.available()) Serial.read();
}