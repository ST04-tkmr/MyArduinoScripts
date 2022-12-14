#include <MsTimer2.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7);
volatile int data,data_avr;
static volatile int data1[3];

void timer1(void);

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);

  MsTimer2::set(1, timer1);
  MsTimer2::start();
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0, 0);
  
  if ((1023 * 0.9f) < data && data < (1023 * 1.1f)) {
    
  } else if ((831 * 0.9f) < data && data < (831 * 1.1f)) {
    lcd.print("SELECT");
  } else if ((628 * 0.9f) < data && data < (628 * 1.1f)) {
    lcd.print("LEFT");
  } else if ((413 * 0.9f) < data && data < (413 * 1.1f)) {
    lcd.print("DOWN");
  } else if ((209 * 0.9f) < data && data < (209 * 1.1f)) {
    lcd.print("UP");
  } else if (data < 20) {
    lcd.print("RIGHT");
  }
    lcd.print("                ");

  lcd.setCursor(0, 1);
 
  lcd.print(data * 0.0032f);
  lcd.print("                ");
}

void timer1(void) {
  data = data_avr;
  data_avr = (data1[2] + data1[1] + data1[0]) / 3;
  data1[2] = data1[1];
  data1[1] = data1[0];
  data1[0] = analogRead(A0);
}
