#include <Wire.h>
#include <MsTimer2.h>
#define RED (4)
#define GREEN (3)
#define YELLOW (2)
#define SW1 (10)
#define SW2 (11)
#define SW3 (12)
#define SW4 (13)
#define READ (1)
#define WRITE (0)

volatile unsigned char LAST_SW1_ON,LAST_SW2_ON,LAST_SW3_ON,LAST_SW4_ON;
volatile unsigned char SW1_ON,SW2_ON,SW3_ON,SW4_ON;
volatile unsigned char chatt1[3],chatt2[3],chatt3[3],chatt4[3];
volatile unsigned char swFlag,incFlag,decFlag,changeFlag;
volatile unsigned char led;
char state1,state2;
volatile char i2c_address = 0x50;
volatile unsigned short address;
volatile unsigned char quantity;
volatile unsigned short value;

void timer1(void);

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);

  MsTimer2::set(1, timer1);
  MsTimer2::start();

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SW4, INPUT);

  led = 0b000;
  state1 = READ;
  state2 = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (swFlag == 1) {
    switch (state1) {
      case READ:
        switch (state2) {
          case 0:
            address = led;
            state2++;
            led = 0b000;
            Serial.println("");
            Serial.print("address : ");
            Serial.println(address, HEX);
            digitalWrite(RED, (led >> 2) & 0x01);
            digitalWrite(GREEN, (led >> 1) & 0x01);
            digitalWrite(YELLOW, led & 0x01);
            break;
          case 1:
            if (led > 0b000) {
              quantity = led;
              state2++;
              led = 0b000;
              Serial.print("quantity : ");
              Serial.println(quantity, HEX);
              digitalWrite(RED, (led >> 2) & 0x01);
              digitalWrite(GREEN, (led >> 1) & 0x01);
              digitalWrite(YELLOW, led & 0x01);
            }
            break;
          case 2:
            Wire.beginTransmission(i2c_address);
            Wire.write(address >> 8);
            Wire.write(address & 0xFF);
            Wire.endTransmission();
            for (int i = address; i < address + quantity; i++) {
              Wire.requestFrom(i2c_address, 1);
              while (Wire.available()) {
                Serial.print(i, HEX);
                Serial.print(" : ");
                Serial.println(Wire.read(), HEX);
              }
            }
            state2 = 0;
            break;
        }
        break;
      case WRITE:
        switch (state2) {
          case 0:
            address = led;
            state2++;
            led = 0b000;
            Serial.println("");
            Serial.print("address : ");
            Serial.println(address, HEX);
            digitalWrite(RED, (led >> 2) & 0x01);
            digitalWrite(GREEN, (led >> 1) & 0x01);
            digitalWrite(YELLOW, led & 0x01);
            break;
          case 1:
            value = led;
            state2++;
            led = 0b000;
            Serial.print("value : ");
            Serial.println(value, HEX);
            digitalWrite(RED, (led >> 2) & 0x01);
            digitalWrite(GREEN, (led >> 1) & 0x01);
            digitalWrite(YELLOW, led & 0x01);
            break;
          case 2:
            Wire.beginTransmission(i2c_address);
            Wire.write(address >> 8);
            Wire.write(address & 0xFF);
            Wire.write(value);
            Wire.endTransmission();
            delay(5);
            Serial.println("completed Writing");
            state2 = 0;
            break;
        }
        break;
    }
    swFlag = 0;
  }

  if (changeFlag == 1 && state2 == 0) {
    Serial.println("");
    switch (state1) {
      case READ:
        state1 = WRITE;
        Serial.println("MODE : WRITE");
        break;
      case WRITE:
        state1 = READ;
        Serial.println("MODE : READ");
        break;
    }
    changeFlag = 0;
  }

  if (incFlag == 1) {
    if (led < 0b111 && state2 != 2) {
      led++;
      digitalWrite(RED, (led >> 2) & 0x01);
      digitalWrite(GREEN, (led >> 1) & 0x01);
      digitalWrite(YELLOW, led & 0x01);
    }
    incFlag = 0;
  }

  if (decFlag == 1) {
    if (led > 0b000 && state2 != 2) {
      led--;
      digitalWrite(RED, (led >> 2) & 0x01);
      digitalWrite(GREEN, (led >> 1) & 0x01);
      digitalWrite(YELLOW, led & 0x01);
    }
    decFlag = 0;
  }
}

void timer1(void) {
  LAST_SW1_ON = SW1_ON;
  SW1_ON = (chatt1[2] & chatt1[1] & chatt1[0]);
  chatt1[2] = chatt1[1];
  chatt1[1] = chatt1[0];
  chatt1[0] = (~digitalRead(SW1) & 0x01);

  LAST_SW2_ON = SW2_ON;
  SW2_ON = (chatt2[2] & chatt2[1] & chatt2[0]);
  chatt2[2] = chatt2[1];
  chatt2[1] = chatt2[0];
  chatt2[0] = (~digitalRead(SW2) & 0x01);

  LAST_SW3_ON = SW3_ON;
  SW3_ON = (chatt3[2] & chatt3[1] & chatt3[0]);
  chatt3[2] = chatt3[1];
  chatt3[1] = chatt3[0];
  chatt3[0] = (~digitalRead(SW3) & 0x01);

  LAST_SW4_ON = SW4_ON;
  SW4_ON = (chatt4[2] & chatt4[1] & chatt4[0]);
  chatt4[2] = chatt4[1];
  chatt4[1] = chatt4[0];
  chatt4[0] = (~digitalRead(SW4) & 0x01);

  if (LAST_SW4_ON == 0 && SW4_ON == 1) {
    incFlag = 1;
  } else if (LAST_SW3_ON == 0 && SW3_ON == 1) {
    decFlag = 1;
  }

  if (LAST_SW2_ON == 0 && SW2_ON == 1) {
    changeFlag = 1;
  }

  if (LAST_SW1_ON == 0 && SW1_ON == 1) {
    swFlag = 1;
  }
}
