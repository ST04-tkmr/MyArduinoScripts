#include <MsTimer2.h>
#define STANDBY (0)
#define START (1)
#define GU (6)
#define TYOKI (8)
#define PA (10)
#define SWITCH (12)
#define YELLOW (9)
#define RED (11)
#define GREEN (13)

volatile unsigned char SW_ON,GU_ON,TYOKI_ON,PA_ON;
volatile unsigned char LAST_SW_ON,LAST_GU_ON,LAST_TYOKI_ON,LAST_PA_ON;
volatile unsigned char swFlag;
static volatile unsigned char chatt1[3],chatt_GU[3],chatt_TYOKI[3],chatt_PA[3];
char state = STANDBY;
volatile unsigned short count = 0;
unsigned char randNum;
unsigned char ledVal[3];
unsigned char winCount = 0;
unsigned char myHand;


void timer1(void);
char judge(unsigned char hand,unsigned char otherHand);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(GU, INPUT);
  pinMode(TYOKI, INPUT);
  pinMode(PA, INPUT);
  pinMode(SWITCH, INPUT);

  MsTimer2::set(1,timer1);
  MsTimer2::start();
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case STANDBY:
      if (swFlag == 1) {
        swFlag = 0;
        state = START;
        Serial.println("");
        Serial.println("じゃんけん...");
      }
      break;
    case START:
      if (swFlag == 1) {
        swFlag = 0;
        state = STANDBY;
        Serial.println("ぽい！");
        judge(myHand,randNum);
      }
      break;
  }

  digitalWrite(YELLOW,ledVal[0]);
  digitalWrite(RED,ledVal[1]);
  digitalWrite(GREEN,ledVal[2]);
}

void timer1(void) {
    LAST_SW_ON = SW_ON;
    SW_ON = (chatt1[2] & chatt1[1] & chatt1[0]);
    chatt1[2] = chatt1[1];
    chatt1[1] = chatt1[0];

    
    LAST_GU_ON = GU_ON;
    GU_ON = (chatt_GU[2] & chatt_GU[1] & chatt_GU[0]);
    chatt_GU[2] = chatt_GU[1];
    chatt_GU[1] = chatt_GU[0];

    LAST_TYOKI_ON = TYOKI_ON;
    TYOKI_ON = (chatt_TYOKI[2] & chatt_TYOKI[1] & chatt_TYOKI[0]);
    chatt_TYOKI[2] = chatt_TYOKI[1];
    chatt_TYOKI[1] = chatt_TYOKI[0];

    LAST_PA_ON = PA_ON;
    PA_ON = (chatt_PA[2] & chatt_PA[1] & chatt_PA[0]);
    chatt_PA[2] = chatt_PA[1];
    chatt_PA[1] = chatt_PA[0];
    
  if (state == STANDBY) {
    chatt1[0] = (~digitalRead(SWITCH) & 0x01);
  } else if (state == START) {
    chatt_GU[0] = (~digitalRead(GU) & 0x01);

    chatt_TYOKI[0] = (~digitalRead(TYOKI) & 0x01);

    chatt_PA[0] = (~digitalRead(PA) & 0x01);
  }

  if (LAST_SW_ON == 0 && SW_ON == 1) {
    swFlag = 1;
  } else if (LAST_GU_ON == 0 && GU_ON == 1) {
    swFlag = 1;
    myHand = 0;
  } else if (LAST_TYOKI_ON == 0 && TYOKI_ON == 1) {
    swFlag = 1;
    myHand = 1;
  } else if (LAST_PA_ON == 0 && PA_ON == 1) {
    swFlag = 1;
    myHand = 2;
  }

  if (state == START) count++;
  if (count == 50) {
    count = 0;
    //相手の手をランダムに動かす
    randNum = rand() % 3;
    switch (randNum) {
      case 0: //グー
        ledVal[0] = 1;
        ledVal[1] = 0;
        ledVal[2] = 0;
        break;
      case 1://チョキ
        ledVal[0] = 0;
        ledVal[1] = 1;
        ledVal[2] = 0;
        break;
      case 2://パー
        ledVal[0] = 0;
        ledVal[1] = 0;
        ledVal[2] = 1;
        break;
    }
  }
}

char judge(unsigned char hand,unsigned char otherHand) {
  char result = (hand - otherHand + 3) % 3;
  switch (hand) {
    case 0:
      Serial.print("あなたはグー、");
      break;
    case 1:
      Serial.print("あなたはチョキ、");
      break;
    case 2:
      Serial.print("あなたはパー、");
      break;
  }
  switch (otherHand) {
    case 0:
      Serial.print("相手はグー、");
      break;
    case 1:
      Serial.print("相手はチョキ、");
      break;
    case 2:
      Serial.print("相手はパー、");
      break;
  }
  switch (result) {
    case 0:
      Serial.println("あいこです");
      break;
    case 1:
      Serial.println("あなたの負けです");
      Serial.print("連勝記録：");
      Serial.print(winCount);
      Serial.println("回");
      winCount = 0;
      break;
    case 2:
      Serial.println("あなたの勝ちです");
      winCount++;
      Serial.print(winCount);
      Serial.println("連勝中");
      break;
  }
  return result;
}
