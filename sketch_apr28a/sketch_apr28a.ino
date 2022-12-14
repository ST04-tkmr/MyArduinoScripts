#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);

  int i2c_address = 0x50;
  unsigned int address = 0x00;

  Wire.beginTransmission(i2c_address);
  Wire.write((unsigned int)(address >> 8));
  Wire.write((unsigned int)(address & 0xFF));
  Wire.endTransmission();
  /*
  Wire.requestFrom(i2c_address, 8);
  while (Wire.available()) {
    Serial.println(Wire.read(), HEX);
  }
  */
  for (int i = address; i < address + 64; i++) {
    Wire.requestFrom(i2c_address, 1);
    while (Wire.available()) {
      Serial.print(i, HEX);
      Serial.print(" : ");
      Serial.println(Wire.read(), HEX);
    }
  }
}

void loop() {
}
