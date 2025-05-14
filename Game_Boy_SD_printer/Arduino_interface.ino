/* Game Boy Printer Interface with Arduino
 * Author: Raphaël Boichot
 * Date: 2023-04-03
 *
 * This code interfaces any program with a Pocket Printer via serial port.
 * Communication alternates between sending and receiving bytes.
 *
 * Protocol:
 *  1. Send INIT command (valid for at least 10 seconds).
 *  2. Wait 200 ms.
 *  3. Send DATA packets.
 *  4. Send an empty DATA packet (no payload).
 *  5. Send PRINT command.
 *  6. Send INQU commands until the print is finished.
 */

char receivedByte;
bool bitSent, bitReceived;
const int CLOCK_PIN = 2;    // Clock signal
const int DATA_TX_PIN = 3;  // Data from Arduino to Printer
const int DATA_RX_PIN = 4;  // Data from Printer to Arduino
const int LED_PIN = 13;     // LED indicates printer ready state

void setup() {
  // Configure pins
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_TX_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(DATA_RX_PIN, INPUT_PULLUP);

  // Initialize pin states
  digitalWrite(CLOCK_PIN, HIGH);
  digitalWrite(DATA_TX_PIN, LOW);

  // Start serial communication
  Serial.begin(115200);
  while (!Serial) {
    ;  // Wait for Serial to initialize
  }

  Serial.println("Waiting for data");

  // Flush any remaining data in the Serial buffer
  while (Serial.available() > 0) {
    Serial.read();
  }

  // Indicate that the printer is ready
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  // Check if data is available on the Serial port
  if (Serial.available() > 0) {
    char byteToSend = Serial.read();
    char response = transferByte(byteToSend);
    Serial.write(response);
  }
}

/* Function to transfer a byte to the printer and receive a response */
char transferByte(char byteToSend) {
  for (int bitPosition = 0; bitPosition < 8; bitPosition++) {
    // Extract the current bit to send
    bitSent = bitRead(byteToSend, 7 - bitPosition);

    // Send the bit
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_TX_PIN, bitSent);
    digitalWrite(LED_PIN, bitSent);
    delayMicroseconds(30);  // Double speed mode

    // Receive the bit
    digitalWrite(CLOCK_PIN, HIGH);
    bitReceived = digitalRead(DATA_RX_PIN);
    bitWrite(receivedByte, 7 - bitPosition, bitReceived);
    delayMicroseconds(30);  // Double speed mode
  }

  // Optional delay between bytes (can be less than 1490 µs)
  delayMicroseconds(0);
  return receivedByte;
}