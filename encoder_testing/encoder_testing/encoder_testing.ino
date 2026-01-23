#include <SPI.h>

#define CS_PIN 5           // Chip select connected to digital pin 2
#define AMT22_NOP 0x00     // No-operation byte per datasheet
#define NUM_POSITIONS_PER_REV 16384  // 2^14 bit encoder
#define AMT22_ZERO 0x70

void setup() {
  SPI.begin();                 // Initialize SPI bus
  pinMode(CS_PIN, OUTPUT);     // Set chip select pin
  digitalWrite(CS_PIN, HIGH);  // Default CS high (inactive)
  Serial.begin(115200);        // Initialize serial output
  setZeroSPI(CS_PIN);
}

void loop() {
  uint16_t position_14bit = readEncoderPosition14Bit();
  float position_float = encoderReadingToDeg(position_14bit);
  Serial.print("Position (14 bit): ");
  Serial.print(position_14bit, DEC);            // Print absolute position value
  Serial.print(" = ");
  Serial.print(position_float, DEC);
  Serial.println(" deg");
  delay(100);                          // Update rate
}

uint16_t readEncoderPosition14Bit(void) {
  uint16_t position = 0;

  digitalWrite(CS_PIN, LOW);           // Begin SPI: CS Low
  delayMicroseconds(3);                // Wait >= 3us per protocol

  position = SPI.transfer(AMT22_NOP);  // First byte (High byte)
  position = position << 8;
  delayMicroseconds(3);

  position |= SPI.transfer(AMT22_NOP); // Second byte (Low byte)
  digitalWrite(CS_PIN, HIGH);          // End SPI: CS High

  // Mask for upper two checksum bits (position valid bits are 0-13)
  position &= 0x3FFF;
  return position;
}

float encoderReadingToDeg(uint16_t position) {
  return 360 * ((float)position / (NUM_POSITIONS_PER_REV-1));
}

float readEncoderPositionDeg(void) {
  return encoderReadingToDeg(readEncoderPosition14Bit());
}

/*
 * Using the equation on the datasheet we can calculate the checksums and then make sure they match what the encoder sent.
 */
bool verifyChecksumSPI(uint16_t message)
{
  //checksum is invert of XOR of bits, so start with 0b11, so things end up inverted
  uint16_t checksum = 0x3;
  for(int i = 0; i < 14; i += 2)
  {
    checksum ^= (message >> i) & 0x3;
  }
  return checksum == (message >> 14);
}

/*
 * The AMT22 bus allows for extended commands. The first byte is 0x00 like a normal position transfer, but the
 * second byte is the command.
 * This function takes the pin number of the desired device as an input
 */
void setZeroSPI(uint8_t cs_pin)
{
  //set CS to low
  digitalWrite(cs_pin, LOW);
  delayMicroseconds(3);

  //send the first byte of the command
  SPI.transfer(AMT22_NOP);
  delayMicroseconds(3);

  //send the second byte of the command
  SPI.transfer(AMT22_ZERO);
  delayMicroseconds(3);
  
  //set CS to high
  digitalWrite(cs_pin, HIGH);

  delay(250); //250 millisecond delay to allow the encoder to reset
}
