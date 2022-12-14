#include <Wire.h>
#include <skADT7410.h>

#define SENS_ADRS (0x48)
#define ROM_ADRS (0x50)

skADT7410 Temp(SENS_ADRS);

volatile unsigned short address = 0x00;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  
  Wire.beginTransmission(ROM_ADRS);
  Wire.write(address >> 8);
  Wire.write(address & 0xFF);
  Wire.endTransmission();

  for(int i=address;i<address+8;i++) {
    Wire.requestFrom(ROM_ADRS, 1);
    while (Wire.available()) {
      Serial.print(i, HEX);
      Serial.print(" : ");
      Serial.println(Wire.read(), HEX);
    }
  }

  unsigned char ans;
  ans = Temp.Begin();
  if (ans == 0) {
    Serial.println("Initialization normal");
  } else {
    Serial.print("Initialization abnormal ans=");
    Serial.println(ans);
  }
  Temp.ActionMode(ADT_MODE_CONTINUE);
  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned char ans;
  float temp;
  ans = Temp.Read(&temp);
  if (ans == 0) {
    Serial.print(temp, 4);
    Serial.write(0xDF);
    Serial.println("C");
  } else {
    Serial.println("NG");
  }

  Wire.beginTransmission(ROM_ADRS);
  Wire.write(address >> 8);
  Wire.write(address & 0xFF);
  Wire.endTransmission();

  for(int i=address;i<address+4;i++) {
    Wire.requestFrom(ROM_ADRS, 1);
    while (Wire.available()) {
      Serial.print(i, HEX);
      Serial.print(" : ");
      Serial.println(Wire.read(), HEX);
    }
  }
  delay(3000);
}
