#ifndef _TAKAMURA_h
#define _TAKAMURA_h

//それぞれのボタンを押したときのanalogRead(A0)の値
#define SELECT_DATA (831)
#define LEFT_DATA (628)
#define DOWN_DATA (413)
#define UP_DATA (209)
#define RIGHT_DATA (20)//実際は0だが20より小さければRIGHTが押されたと判定
#define NONE_DATA (1023)//何も押されてないとき

#define COUNT_MIN (0)//count変数の最小値
#define COUNT_MAX (9999999999999999)//count変数の最大値

//それぞれの機能の状態
#define START (1)
#define STANDBY (0)

//初期画面に戻る
void _init(void);

//MsTimer2用
void flash(void);

//モードを選ぶ
void modeSelect(void);

//カウンター
void counter(void);

//ストップウォッチの初期設定
void setStopWatch(void);

//ストップウォッチ
void stopWatch(void);

void setTimer(void);

//タイマー
void timer(void);

void setClockView(void);

//時計表示
void clockView(void);

void timeEdit(volatile unsigned long* countPointer, const char* funcName);

//時間の単位h,m,s,msecを表示
void setTimeView(char y);

/*
   カウント（合計ミリ秒）から時間を計算して表示
   y=0で上の列、y=1で下の列に表示
   viewAll=1でcom_viewFlagを無視してhour,minute,sec,msecすべて表示
*/
void timeView(unsigned long count, char y, unsigned char viewAll);

#endif
