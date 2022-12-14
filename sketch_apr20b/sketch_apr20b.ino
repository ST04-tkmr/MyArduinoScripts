#include <MsTimer2.h>

unsigned char inPin = 12;
char pinNum = 3;
unsigned char led[3];
unsigned char outPin[] = {13, 11, 9};
volatile unsigned char SW_ON = false;
volatile unsigned char LAST_SW_ON;
volatile unsigned char swFlag;
static volatile unsigned char chatt1[3];
char state = 0;

void timer1(void);
void halfAdd(unsigned char *s, unsigned char *c, unsigned char a, unsigned char b);
void fullAdd(unsigned char *s, unsigned char *c, unsigned char a, unsigned char b,unsigned char x);
void counter(unsigned char *p);

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  for (int i=0;i<pinNum;i++) {
    pinMode(outPin[i], OUTPUT);
  }
  pinMode(inPin, INPUT);

  MsTimer2::set(1, timer1);
  MsTimer2::start();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (swFlag == 1 && state == 0) {
    swFlag = 0;
    state = 1;
  } else if (swFlag == 1 && state == 1) {
    swFlag = 0;
    state = 0;
  }
  if (state == 1) {
    for (int i=0;i<pinNum;i++) {
      digitalWrite(outPin[i], led[i]);
    }
    counter(led);
    delay(500);
    Serial.println("Hello 世界");
  }
}

void timer1(void) {
  LAST_SW_ON = SW_ON;
  SW_ON = (chatt1[2] & chatt1[1] & chatt1[0]);
  chatt1[2] = chatt1[1];
  chatt1[1] = chatt1[0];
  chatt1[0] = (~digitalRead(inPin) & 0x01);

  if (LAST_SW_ON == 0 && SW_ON == 1) {
    swFlag = 1;
  }
}

void halfAdd(unsigned char *s, unsigned char *c, unsigned char a, unsigned char b) {
  unsigned char a0,b0;
  a0 = a & (1 << 0);
  b0 = b & (1 << 0);

  *s = a0 ^ b0;
  *c = a0 & b0;
}

void fullAdd(unsigned char *s, unsigned char *c, unsigned char a, unsigned char b,unsigned char x) {
  unsigned char a0,b0,x0,s1,s2,c1,c2;
  a0 = a & (1 << 0);
  b0 = b & (1 << 0);
  x0 = x & (1 << 0);

  halfAdd(&s1, &c1, a0, b0);

  halfAdd(&s2, &c2, s1, x0);

  *s = s2;
  *c = c1 | c2;
}

void counter(unsigned char *p) {
  unsigned char s,c,x,ab,bb;
  unsigned char ans[pinNum];
  unsigned char addNum[3] = {0, 0, 1};
  c = 0;

  ab = p[2];
  bb = addNum[2];
  x = c;
  fullAdd(&s, &c, ab, bb, x);
  p[2] = s;

  ab = p[1];
  bb = addNum[1];
  x = c;
  fullAdd(&s, &c, ab, bb, x);
  p[1] = s;

  ab = p[0];
  bb = addNum[0];
  x = c;
  fullAdd(&s, &c, ab, bb, x);
  p[0] = s;
}
