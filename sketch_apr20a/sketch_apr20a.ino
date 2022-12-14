#include <MsTimer2.h>

int ledPin = 13;
int inPin = 12;
int state = 0;
volatile unsigned char SW_ON;
volatile unsigned char LAST_SW_ON;
static volatile unsigned char chatt1[3];
volatile unsigned char swFlag;

void flash(void) {
  LAST_SW_ON = SW_ON;
  SW_ON = ( chatt1[2] & chatt1[1] & chatt1[0]);
  chatt1[2] = chatt1[1];
  chatt1[1] = chatt1[0];
  chatt1[0] = ( ~digitalRead(inPin) & 0x01);

  if ((LAST_SW_ON == 0) && (SW_ON == 1))
  {
    swFlag = 1;
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  pinMode(inPin, INPUT);
  

  MsTimer2::set(1,flash); // 500msごとにオンオフ
  MsTimer2::start();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (swFlag == 1 && state == 0)
  {
    swFlag = 0;
    state = 1;
    digitalWrite(ledPin, HIGH);
  }
  else if (swFlag == 1 && state == 1)
  {
    swFlag = 0;
    state = 0;
    digitalWrite(ledPin, LOW);
  }
}
