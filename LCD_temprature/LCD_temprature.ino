#include <skADT7410.h>
#include <Wire.h>

#define LCD_ADRS (0x3E)
#define SENS_ADRS (0x48)

skADT7410 Temp(SENS_ADRS);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  init_LCD();

  unsigned char ans;
  ans = Temp.Begin();
  if (ans == 0) {
    Serial.println("skADT7410 Initialization normal");
  } else {
    Serial.print("skADT7410 Initialization abnormal ans=");
    Serial.println(ans);
  }
  Serial.println("");
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

    int intPart = (int)temp;
    writeData(0x30 + (char)(intPart / 10));
    writeData(0x30 + (char)(intPart % 10));
    writeData(0x2E);
    unsigned int deciPart = (temp - (int)temp) * 1000;
    writeData(0x30 + (char)(deciPart / 100));
    writeData(0x30 + (char)((deciPart % 100) / 10));
    writeData(0x30 + (char)((deciPart % 100) % 10));
    writeData(0xDF);
    writeData(0x43);
  } else {
    Serial.println("NG");

    //NG
    writeData(0x4E);
    writeData(0x47);
  }
  delay(1000);

  //clear display
  writeCommand(0x01);

  //set DDRAM address
  writeCommand(0x80 + 0x00);
}

void init_LCD() {
  delay(100);
  writeCommand(0x38);
  delay(20);
  writeCommand(0x39);
  delay(20);
  writeCommand(0x14);
  delay(20);
  writeCommand(0x73);
  delay(20);
  writeCommand(0x52);
  delay(20);
  writeCommand(0x6C);
  delay(20);
  writeCommand(0x38);
  delay(20);
  writeCommand(0x01);
  delay(20);
  writeCommand(0x0C);
  delay(20);
}

void writeData(byte t_data){
  Wire.beginTransmission(LCD_ADRS);
  Wire.write(0x40);
  Wire.write(t_data);
  Wire.endTransmission();
  delay(1);
}

void writeCommand(byte t_command) {
  Wire.beginTransmission(LCD_ADRS);
  Wire.write(0x00);
  Wire.write(t_command);
  Wire.endTransmission();
  delay(10);
}
