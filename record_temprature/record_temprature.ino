#include <MsTimer2.h>
#include <Wire.h>
#include <skADT7410.h>

#define SENS_ADRS (0x48)//ADT7410のアドレス
#define ROM_ADRS (0x50)//24LC256のアドレス
#define LOG_SIZE (8)
#define PARAM_SIZE (8)
#define PARAM_TOP_ADRS (0x00)
#define LOG_TOP_ADRS (0x08)
#define LOG_MAX (32767)

skADT7410 Temp(SENS_ADRS);

typedef volatile unsigned char v_uchar;
typedef volatile unsigned short v_ushort;

typedef union {
  v_uchar BUFF[LOG_SIZE];
  struct {
    v_uchar hour;
    v_uchar minute;
    v_ushort sec;
    volatile short int_part;//温度の整数部分
    v_ushort deci_part;//温度の小数部分
  }DATA;
}tagLOG;

//tagLOG初期化関数
tagLOG initTagLOG(v_uchar iniTime[3], volatile short ini_int_part, v_ushort ini_deci_part) {
  tagLOG tl;
  tl.DATA.hour = iniTime[1];
  tl.DATA.minute = iniTime[2];
  tl.DATA.sec = iniTime[3];
  tl.DATA.int_part = ini_int_part;
  tl.DATA.deci_part = ini_deci_part;
  return tl;
}

typedef union {
  v_uchar BUFF[PARAM_SIZE];
  struct {
    v_uchar check;//前回データを書き込んだか確認するための変数
    v_ushort read_adrs;//次に読み込むときの開始番地
    v_ushort write_adrs;//次に書き込むときの開始番地
    v_ushort data_quantity;//書き込んであるデータ個数
    v_uchar dataMaxFlag;//書き込めるデータ量の最大に達したら1
  }DATA;
}tagPARAM;

//tagPARAM初期化関数
tagPARAM initTagPARAM(v_uchar check, v_ushort read_adrs, v_ushort write_adrs, v_ushort data_quantity, v_uchar dataMaxFlag) {
  tagPARAM tp;
  tp.DATA.check = check;
  tp.DATA.read_adrs = read_adrs;
  tp.DATA.write_adrs = write_adrs;
  tp.DATA.data_quantity = data_quantity;
  tp.DATA.dataMaxFlag = dataMaxFlag;
  return tp;
}

//スイッチの構造体
typedef struct {
  v_uchar pin;
  v_uchar swFlag;
  v_uchar LAST_SW_ON;
  v_uchar SW_ON;
  v_uchar chatt[3];
}SW;

tagLOG initTagLOG(v_uchar* iniTime, volatile short ini_int_part, v_ushort ini_deci_part);
tagPARAM initTagPARAM(v_uchar check, v_ushort read_adrs, v_ushort write_adrs, v_ushort data_quantity, v_uchar dataMaxFlag);
unsigned char checkLastWrite(tagPARAM* param);
unsigned int secretCalc(tagPARAM* param);
unsigned char countOnes(unsigned int n);
void confirmSW(SW* s);

SW read_sw,write_sw;
tagLOG setLog;
tagPARAM setParam;

volatile unsigned long count;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();

  MsTimer2::set(1, flash);
  MsTimer2::start();

  read_sw.pin = 11;
  write_sw.pin = 12;

  v_uchar iniTime[] = {0, 0, 0};
  setLog = initTagLOG(iniTime, 0, 0);

  //param用に用意する領域のデータを読み込む
  tagPARAM param;
  Wire.beginTransmission(ROM_ADRS);
  Wire.write((PARAM_TOP_ADRS >> 8) & 0xFF);
  Wire.write(PARAM_TOP_ADRS & 0xFF);
  Wire.endTransmission();
  for (short i=PARAM_TOP_ADRS;i<PARAM_TOP_ADRS+PARAM_SIZE;i++) {
    Wire.requestFrom(ROM_ADRS, 1);
    while (Wire.available()) {
      param.BUFF[i] = Wire.read();
      //Serial.println(param.BUFF[i], HEX);
    }
  }
  //param用の領域のデータから前回書き込みを行ったことが確認できなかったら初期化、できたらデータを引き継ぐ
  unsigned char check_ans;
  check_ans = checkLastWrite(&param);
  if (check_ans == 0) {
    setParam = param;
    Serial.println("getting PARAM is successed");
  } else {
    v_uchar check;
    unsigned int num = LOG_TOP_ADRS * LOG_TOP_ADRS + 0;
    check = countOnes(num);
    setParam = initTagPARAM(check, LOG_TOP_ADRS, LOG_TOP_ADRS, 0, 0);
    Serial.println("PARAM Initialization");
  }
  Serial.println("PARAM {");
  Serial.print("check : ");
  Serial.println(setParam.DATA.check);
  Serial.print("read_adrs : ");
  Serial.println(setParam.DATA.read_adrs);
  Serial.print("write_adrs : ");
  Serial.println(setParam.DATA.write_adrs);
  Serial.print("data_quantity : ");
  Serial.println(setParam.DATA.data_quantity);
  Serial.print("dataMaxFlag : ");
  Serial.println(setParam.DATA.dataMaxFlag);
  Serial.println("}");
  Serial.println("");

  //skADT7410クラスの初期化
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
  if (read_sw.swFlag == 1) {
    Serial.println("Read {");
    if (setParam.DATA.data_quantity > 0) {
      tagLOG LOG;
      v_ushort adrs = setParam.DATA.read_adrs;
      for (int i=0;i<setParam.DATA.data_quantity;i++) {
        if (adrs > LOG_MAX - LOG_SIZE) {
          adrs = LOG_TOP_ADRS;
        }
        Wire.beginTransmission(ROM_ADRS);
        Wire.write((adrs >> 8) & 0xFF);
        Wire.write(adrs & 0xFF);
        Wire.endTransmission();
        for (int j=0;j<LOG_SIZE;j++) {
          Wire.requestFrom(ROM_ADRS, 1);
          while (Wire.available()) {
            LOG.BUFF[j] = Wire.read();
          }
        }
        Serial.print(LOG.DATA.hour);
        Serial.print(" : ");
        Serial.print(LOG.DATA.minute);
        Serial.print(" : ");
        Serial.print(LOG.DATA.sec);
        Serial.print("   ");
        Serial.print(LOG.DATA.int_part);
        Serial.print(".");
        Serial.print(LOG.DATA.deci_part);
        Serial.println("℃");
        adrs += LOG_SIZE;
      }
    } else {
      Serial.println("data is empty");
    }
    Serial.println("}");
    Serial.println("");
    read_sw.swFlag = 0;
  }

  if (write_sw.swFlag == 1) {
    unsigned char ans;
    float temp;
    ans = Temp.Read(&temp);
    Serial.println("Write {");
    if (ans == 0) {
      v_ushort adrs = setParam.DATA.write_adrs;
      setParam.DATA.write_adrs += LOG_SIZE;
      if (setParam.DATA.dataMaxFlag == 0) {
        setParam.DATA.data_quantity++;
      }
      if (setParam.DATA.write_adrs > LOG_MAX - LOG_SIZE) {
        setParam.DATA.write_adrs = LOG_TOP_ADRS;
        if (setParam.DATA.dataMaxFlag == 0) {
          setParam.DATA.dataMaxFlag = 1;
        }
      }

      if (setParam.DATA.dataMaxFlag == 1) {
        setParam.DATA.read_adrs += LOG_SIZE;
        if (setParam.DATA.read_adrs > LOG_MAX - LOG_SIZE) {
          setParam.DATA.read_adrs = LOG_TOP_ADRS;
        }
      }

      unsigned int a = secretCalc(&setParam);
      setParam.DATA.check = countOnes(a);

      //次回書き込む番地や読み込む番地などを書き込む
      Wire.beginTransmission(ROM_ADRS);
      Wire.write((PARAM_TOP_ADRS >> 8) & 0xFF);
      Wire.write(PARAM_TOP_ADRS & 0xFF);
      for (int i=0;i<PARAM_SIZE;i++) {
        Wire.write(setParam.BUFF[i]);
      }
      Wire.endTransmission();
      delay(5);
      
      setLog.DATA.hour = (count / 1000 / 60 / 60) % 24;
      setLog.DATA.minute = (count / 1000 / 60) % 60;
      setLog.DATA.sec = (count / 1000) % 60;
      setLog.DATA.int_part = (short) temp;
      setLog.DATA.deci_part = (short) ((temp - (short) temp) * 1000);

      Wire.beginTransmission(ROM_ADRS);
      Wire.write((adrs >> 8) & 0xFF);
      Wire.write(adrs & 0xFF);
      for (int i=0;i<LOG_SIZE;i++) {
        Wire.write(setLog.BUFF[i]);
      }
      Wire.endTransmission();
      delay(5);
      
      Serial.print(setLog.DATA.hour);
      Serial.print(" : ");
      Serial.print(setLog.DATA.minute);
      Serial.print(" : ");
      Serial.print(setLog.DATA.sec);
      Serial.print("   ");
      Serial.print(setLog.DATA.int_part);
      Serial.print(".");
      Serial.print(setLog.DATA.deci_part);
      Serial.println("℃");
    } else {
      Serial.println("writing error");
    }
    Serial.println("}");
    Serial.println("");
    write_sw.swFlag = 0;
  }
}

void flash(void) {
  confirmSW(&read_sw);
  confirmSW(&write_sw);

  if (count < 86400000) {
    count++;
  } else {
    count = 0;
  }
}

/*
 * 前回書き込みを行ったかどうかをチェックする
 * 初期状態や別のデータが書き込まれていた場合への対策
 * checkの値が
 * read_adrs * write_adrs + data_quantity
 * の計算結果を2進数で表示したときの1の個数と一致していれば0、そうでなければ1を返す
 */
unsigned char checkLastWrite(tagPARAM* param) {
  unsigned int result = secretCalc(param);
  unsigned char zeroNum = countOnes(result);
  if (param->DATA.check == zeroNum) {
    return 0;
  } else {
    return 1;
  }
}

unsigned int secretCalc(tagPARAM* param) {
  return param->DATA.read_adrs * param->DATA.write_adrs + param->DATA.data_quantity;
}

//1の個数を数えて返す
unsigned char countOnes(unsigned int n) {
  unsigned char count = 0;
  while (n) {
    count += n & 0x01;
    n >>= 1;
  }
  return count;
}

//スイッチが押されていたらswFlag=1にする
void confirmSW(SW* s) {
  s->LAST_SW_ON = s->SW_ON;
  s->SW_ON = (s->chatt[2] & s->chatt[1] & s->chatt[0]);
  s->chatt[2] = s->chatt[1];
  s->chatt[1] = s->chatt[0];
  s->chatt[0] = (~digitalRead(s->pin) & 0x01);
  if (s->LAST_SW_ON == 0 && s->SW_ON == 1) {
    s->swFlag = 1;
  }
}
