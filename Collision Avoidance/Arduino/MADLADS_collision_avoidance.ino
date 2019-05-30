/*
 * MADLADS Collision Avoidance System
 * 
 * Pinout for OSEPP IR Proximity Sensor
 * RED: 5V
 * WHITE:  I2C SDA (pin A4 on Uno; pin 20 on Mega)
 * BLACK: GND
 * GREY: I2C SCL (pin A5 on Uno; pin 21 on Mega)
 * 
 * Functionality:
 *  If the sensor is connected properly, it will:
 *    transmit a "1" when an object is within 0.5 - 5 cm; 
 *    transmit a "0" otherwise
 *  Alternates between "1" and "0" if the sensor is not
 *   configured correctly
 *   
 * Code Acknowledgements: OSEPP
 */

#include <Wire.h>

#define SENSOR_ADDR_OFF_OFF (0x26)
#define SENSOR_ADDR_OFF_ON (0x22)
#define SENSOR_ADDR_ON_OFF (0x24)
#define SENSOR_ADDR_ON_ON (0x20)
#define PROXIMITY_OUTPUT_PIN 8

// Set the sensor address here
const uint8_t sensorAddr = SENSOR_ADDR_OFF_OFF;

// OSEPP IR Proximity Sensor Functions (by OSEPP)
int ReadByte(uint8_t addr, uint8_t reg, uint8_t *data);
void WriteByte(uint8_t addr, uint8_t reg, byte data);

void setup() {
  pinMode(PROXIMITY_OUTPUT_PIN, OUTPUT);
  Serial.begin(9600);
  Wire.begin(); 
  WriteByte(sensorAddr, 0x3, 0xFE);
}

void loop() {
  uint8_t val; 
  static unsigned char blinkFlag = 0;
  if (ReadByte(sensorAddr, 0x0, &val) == 0)
  { 
    // The second LSB indicates if something was not detected, i.e.,
    // LO = object detected, HI = nothing detected
    if (val & 0x2)
    {
      // Nothing detected
      digitalWrite(PROXIMITY_OUTPUT_PIN, LOW);
    }
    else
    {
      // Object detected
      digitalWrite(PROXIMITY_OUTPUT_PIN, HIGH);
    }
  }
  else
  {
    // Failed to read from sensor
    if (blinkFlag)
    {
      digitalWrite(PROXIMITY_OUTPUT_PIN, HIGH);
      blinkFlag = 0;
    }
    else
    {
      digitalWrite(PROXIMITY_OUTPUT_PIN, LOW);
      blinkFlag = 1;
    }
  } 
  delay(500);
}

// OSEPP IR Proximity Sensor Function (by OSEPP)
// Read a byte on the i2c interface
int ReadByte(uint8_t addr, uint8_t reg, uint8_t *data) {
  // Do an i2c write to set the register that we want to read from
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(); // Read a byte from the device
  Wire.requestFrom(addr, (uint8_t)1);
  if (Wire.available())
  {
    *data = Wire.read();
  }
  else
  { // Read nothing back
    return -1;
  }
  return 0;
}

// OSEPP IR Proximity Sensor Function (by OSEPP)
// Write a byte on the i2c interface
void WriteByte(uint8_t addr, uint8_t reg, byte data) {
 // Begin the write sequence
 Wire.beginTransmission(addr);; // First byte is to set the register pointer
 Wire.write(reg); // Write the data byte
 Wire.write(data); // End the write sequence; bytes are actually transmitted now
 Wire.endTransmission();
}
