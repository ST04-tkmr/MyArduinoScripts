#include <MsTimer2.h>
#include <LiquidCrystal.h>
#include "clock_takamura.h"//定数とかプロトタイプ宣言とか

//mode変数に格納する値
#define SELECT (0)
#define COUNTER (1)
#define STOPWATCH (2)
#define TIMER (3)
#define CLOCK (4)

//追加ボタンのINPUTのPIN番号
#define INPIN (13)

//LED用OUTPUTのPIN番号
#define OUTPIN (12)

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7);
volatile unsigned char mode;//現在選択中のモードSELECT or COUNTER or STOPWATCH or TIMER or CLOCK
volatile unsigned char nowMode;//選択したモード
volatile unsigned char stopwatch_is;//ストップウォッチの状態START or STANDBY
volatile int data, data_avr;
static volatile int data1[3];
volatile unsigned char LAST_SW_ON, SW_ON;
static volatile unsigned char chatt1[3];
volatile unsigned char swFlag;
volatile unsigned long c_count, s_count, t_count; //counter,stopwatch,timer
volatile unsigned char viewFlag;//1秒ごとにon、画面再表示の回数を抑える

//ストップウォッチ用
static volatile unsigned long lapTime[2];
volatile unsigned long lap;
volatile unsigned char lapCheck;

//タイマー用
volatile unsigned char finFlag;//タイマー終了フラグ
const char* str1 = "RESET";//cursorX=12にしたときの機能名
volatile unsigned long last_count;

//時計用
volatile unsigned long countForEdit;
const char* str2 = "CANCEL";

volatile unsigned long com_count;
volatile unsigned char com_viewFlag;
volatile unsigned char clock_is, clockEditer_is;
volatile unsigned char cursorX;//時間設定のときのカーソルの位置
volatile unsigned char timer_is;

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);

  MsTimer2::set(1, flash);
  MsTimer2::start();

  Serial.begin(9600);

  pinMode(INPIN, INPUT);
  pinMode(OUTPIN, OUTPUT);

  _init();
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (nowMode) {
    case SELECT:
      modeSelect();
      break;
    case COUNTER:
      counter();
      break;
    case STOPWATCH:
      stopWatch();
      break;
    case TIMER:
      timer();
      break;
    case CLOCK:
      clockView();
      break;
  }

  if (nowMode != SELECT) {
    if (swFlag == 1) {
      swFlag = 0;
      nowMode = SELECT;
    }
  } else if (nowMode == SELECT) {
    if (swFlag == 1) {
      swFlag = 0;
      if (finFlag == 1) {
        finFlag = 0;
        digitalWrite(OUTPIN, LOW);
        timer_is = STANDBY;
        t_count = last_count;
      }
    }
  }
}

void _init(void) {
  mode = SELECT;
  nowMode = SELECT;
  c_count = 0;
  s_count = 0;
  t_count = 0;
  stopwatch_is = STANDBY;
  lap = 0;
  lapTime[1] = 0;
  lapTime[0] = 0;
  clock_is = STANDBY;
  clockEditer_is = STANDBY;
  timer_is = STANDBY;
  finFlag = 0;
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  digitalWrite(OUTPIN, LOW);
}

void flash(void) {
  /*
     analogRead(A0)の平均をとる
     data1の数値すべてが何も押してないときの値(1023)を下回ったときのみ平均をとる
  */
  data = data_avr;
  if (data1[2] < NONE_DATA && data1[1] < NONE_DATA && data1[0] < NONE_DATA) {
    data_avr = (data1[2] + data1[1] + data1[0]) / 3;
  } else {
    data_avr = analogRead(A0);
  }
  data1[2] = data1[1];
  data1[1] = data1[0];
  data1[0] = analogRead(A0);

  LAST_SW_ON = SW_ON;
  SW_ON = (chatt1[2] & chatt1[1] & chatt1[0]);
  chatt1[2] = chatt1[1];
  chatt1[1] = chatt1[0];
  chatt1[0] = (~digitalRead(INPIN) & 0x01);

  if (LAST_SW_ON == 0 && SW_ON == 1) swFlag = 1;

  /*
     ストップウォッチのときはcountは1日ごとにリセット
     24hour = 86400000msec
  */
  if (mode == STOPWATCH && stopwatch_is == START) {
    if (86400000 > s_count) {
      s_count++;
    } else {
      s_count = 0;
    }
  }

  /*
     t_countをミリ秒ごとに-1
     カウントが終了したらfinFlag = 1
  */
  if (mode == TIMER && timer_is == START) {
    if (t_count > 0) {
      t_count--;
    } else if (t_count <= 0 && finFlag == 0) {
      t_count = 0;
      finFlag = 1;
      digitalWrite(OUTPIN, HIGH);
    }
  }

  //常設カウント
  if (86400000 > com_count) {
    com_count++;
    if (com_count % 500 == 0) {
      com_viewFlag = 1;
    }
  } else {
    com_count = 0;
  }
}

void modeSelect(void) {
  lcd.setCursor(0, 0);
  lcd.print("SELECT MODE");
  lcd.setCursor(0, 1);
  lcd.print("<");
  lcd.setCursor(15, 1);
  lcd.print(">");

  //現在選択されているモードを表示
  lcd.setCursor(1, 1);
  switch (mode) {
    case SELECT:
      lcd.print("    RESET     ");
      break;
    case COUNTER:
      lcd.print("   COUNTER    ");
      break;
    case STOPWATCH:
      lcd.print("  STOPWATCH   ");
      break;
    case TIMER:
      lcd.print("    TIMER     ");
      break;
    case CLOCK:
      lcd.print("    CLOCK     ");
      break;
  }

  //LEFT,RIGHTを押してmodeを移動
  if ((LEFT_DATA * 0.95f) < data && data < (LEFT_DATA * 1.05f)) {
    if (mode > SELECT) {
      mode--;
    } else {
      mode = CLOCK;
    }
    delay(200);
  } else if (data < RIGHT_DATA) {
    if (mode < CLOCK) {
      mode++;
    } else {
      mode = SELECT;
    }
    delay(200);
  }

  //SELECTを押して決定
  if ((SELECT_DATA * 0.95f) < data && data < (SELECT_DATA * 1.05f)) {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    switch (mode) {
      case SELECT:
        _init();
        break;
      case COUNTER:
        nowMode = COUNTER;
        break;
      case STOPWATCH:
        nowMode = STOPWATCH;
        setStopWatch();
        break;
      case TIMER:
        nowMode = TIMER;
        setTimer();
        break;
      case CLOCK:
        nowMode = CLOCK;
        setClockView();
        break;
    }
    delay(200);
  }
}

void counter(void) {
  lcd.setCursor(0, 0);
  lcd.print(c_count);
  lcd.print("                ");

  if ((DOWN_DATA * 0.95f) < data && data < (DOWN_DATA * 1.05f)) {//DOWNが押されたらcount-1
    if (COUNT_MIN < c_count) {
      c_count--;
      delay(200);
    }
  } else if ((UP_DATA * 0.95f) < data && data < (UP_DATA * 1.05f)) {//UPが押されたらcount+1
    if (COUNT_MAX > c_count) {
      c_count++;
      delay(200);
    }
  } else if ((SELECT_DATA * 0.95f) < data && data < (SELECT_DATA * 1.05f)) {//SELECTが押されたらcountリセット
    c_count = 0;
  }
}

void setStopWatch(void) {
  lapCheck = 1;

  timeView(s_count, 0, 1);

  lap = lapTime[0] - lapTime[1];
  timeView(lap, 1, 1);
}

void stopWatch(void) {
  if (stopwatch_is == START) {
    timeView(s_count, 0, 0);
  }

  //LEFT
  if ((LEFT_DATA * 0.95f) < data && data < (LEFT_DATA * 1.05f)) {
    if (stopwatch_is == START && lapCheck == 1) {
      lapTime[1] = lapTime[0];
      lapTime[0] = s_count;
      lap = lapTime[0] - lapTime[1];
      timeView(lap, 1, 1);

      lapCheck = 0;
    }
  } else if (data > (NONE_DATA * 0.9f)) {
    lapCheck = 1;
  }

  if ((UP_DATA * 0.95f) < data && data < (UP_DATA * 1.05f)) {//UP:ストップウォッチスタート
    stopwatch_is = START;
  } else if ((DOWN_DATA * 0.95f) < data && data < (DOWN_DATA * 1.05f)) {//DOWN:ストップウォッチストップ
    stopwatch_is = STANDBY;
  } else if ((SELECT_DATA * 0.95f) < data && data < (SELECT_DATA * 1.05f)) {//SELECT:ストップウォッチリセット
    s_count = 0;
    setStopWatch();
  } else if (data < RIGHT_DATA) {//RIGHT:ラップタイムのみリセット
    lapTime[1] = 0;
    lapTime[0] = 0;
    lap = 0;
    timeView(lap, 1, 1);
  }
}

void setTimer(void) {
  timeView(t_count, 0, 1);
}

void timer(void) {
  if (timer_is == START) {
    timeView(t_count, 0, 0);

    if ((SELECT_DATA * 0.95f) < data && data < (SELECT_DATA * 1.05f)) {
      delay(200);
      timer_is = STANDBY;

      if (finFlag == 1) {
        t_count = last_count;
        finFlag = 0;
        digitalWrite(OUTPIN, LOW);
        timeView(t_count, 0, 1);
      }
    }
  } else if (timer_is == STANDBY) {
    timeEdit(&t_count, str1);

    if ((SELECT_DATA * 0.95f) < data && data < (SELECT_DATA * 1.05f)) {
      delay(200);

      switch(cursorX) {
        case 12:
          t_count = 0;
          break;
        default:
          timer_is = START;
          last_count = t_count;
          break;
      }

      lcd.setCursor(0, 1);
      lcd.print("                ");

      timeView(t_count, 0, 1);
    }
  }
}

void setClockView(void) {
  timeView(com_count, 0, 1);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  clock_is = START;
  clockEditer_is = STANDBY;
}

void clockView(void) {
  if (clock_is == START) {
    timeView(com_count, 0, 0);

    if ((SELECT_DATA * 0.95f) < data && data < (SELECT_DATA * 1.05f)) {
      countForEdit = com_count;
      clock_is = STANDBY;
      clockEditer_is = START;
      cursorX = 0;
      delay(200);
    }
  }

  if (clockEditer_is == START) {
    timeEdit(&countForEdit, str2);

    if ((SELECT_DATA * 0.95f) < data && data < (SELECT_DATA * 1.05f)) {
      delay(200);

      switch (cursorX) {
        case 12:
          break;
        default:
          com_count = countForEdit;
          break;
      }

      lcd.setCursor(0, 1);
      lcd.print("                ");

      timeView(com_count, 0, 1);

      clockEditer_is = STANDBY;
      clock_is = START;
    }
  }
}

void timeEdit(volatile unsigned long* countPointer, const char* funcName) {
  if (cursorX != 12) {
    lcd.setCursor(cursorX, 1);
    lcd.print("^");
  } else {
    lcd.setCursor(0, 1);
    lcd.print(funcName);
  }

  if ((LEFT_DATA * 0.95f) < data && data < (LEFT_DATA * 1.05f)) {
    lcd.setCursor(0, 1);
    lcd.print("                ");

    if (cursorX > 0) {
      cursorX -= 3;
    } else {
      cursorX = 12;
    }

    if (cursorX != 12) {
      lcd.setCursor(cursorX, 1);
      lcd.print("^");
    } else {
      lcd.setCursor(0, 1);
      lcd.print(funcName);
    }

    delay(200);
  } else if (data < RIGHT_DATA) {
    lcd.setCursor(0, 1);
    lcd.print("                ");

    if (cursorX < 12) {
      cursorX += 3;
    } else {
      cursorX = 0;
    }

    if (cursorX != 12) {
      lcd.setCursor(cursorX, 1);
      lcd.print("^");
    } else {
      lcd.setCursor(0, 1);
      lcd.print(funcName);
    }

    delay(200);
  }

  if ((DOWN_DATA * 0.95f) < data && data < (DOWN_DATA * 1.05f)) {
    switch (cursorX) {
      case 0://hour
        if ((*countPointer / 1000 / 60 / 60) % 24 > 0) {
          *countPointer -= 3600000;
        }
        delay(200);
        break;
      case 3:
        if ((*countPointer / 1000 / 60) % 60 > 0) {
          *countPointer -= 60000;
        }
        delay(200);
        break;
      case 6:
        if ((*countPointer / 1000) % 60 > 0) {
          *countPointer -= 1000;
        }
        delay(200);
        break;
      case 9:
        if (*countPointer % 1000 > 0) {
          *countPointer--;
        }
        delay(100);
        break;
    }
    timeView(*countPointer, 0, 1);
  } else if ((UP_DATA * 0.95f) < data && data < (UP_DATA * 1.05f)) {
    switch (cursorX) {
      case 0:
        if ((*countPointer / 1000 / 60 / 60) % 24 < 23) {
          *countPointer += 3600000;
        }
        delay(200);
        break;
      case 3:
        if ((*countPointer / 1000 / 60) % 60 < 59) {
          *countPointer += 60000;
        }
        delay(200);
        break;
      case 6:
        if ((*countPointer / 1000) % 60 < 59) {
          *countPointer += 1000;
        }
        delay(200);
        break;
      case 9:
        if (*countPointer % 1000 < 999) {
          *countPointer++;
        }
        delay(100);
        break;
    }
    timeView(*countPointer, 0, 1);
  }
}

void setTimeView(char y) {
  lcd.setCursor(0, y);
  lcd.print("  h  m  s   msec");
}

void timeView(unsigned long count, char y, unsigned char viewAll) {
  volatile unsigned char hour, minute, sec;
  volatile unsigned short msec;

  if (viewAll == 1) {
    msec = count % 1000;
    sec = (count / 1000) % 60;
    minute = (count / 1000 / 60) % 60;
    hour = (count / 1000 / 60 / 60) % 24;
    setTimeView(y);
    lcd.setCursor(9, y);
    lcd.print(msec);
    lcd.setCursor(6, y);
    lcd.print(sec);
    lcd.setCursor(3, y);
    lcd.print(minute);
    lcd.setCursor(0, y);
    lcd.print(hour);
  } else {
    if (com_viewFlag == 1) {
      sec = (count / 1000) % 60;
      minute = (count / 1000 / 60) % 60;
      hour = (count / 1000 / 60 / 60) % 24;
      setTimeView(y);
      lcd.setCursor(6, y);
      lcd.print(sec);
      lcd.setCursor(3, y);
      lcd.print(minute);
      lcd.setCursor(0, y);
      lcd.print(hour);

      com_viewFlag = 0;
    }
    msec = count % 1000;
    lcd.setCursor(9, y);
    lcd.print("   ");
    lcd.setCursor(9, y);
    lcd.print(msec);
  }
}
