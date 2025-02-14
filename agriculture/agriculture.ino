#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

#define MOTOR_ON_AT  40
#define MOTOR_OFF_AT 80
#define number "01744125719"
#define fireMsg "Fire detected!"
#define animalMsg "Animal detected!"

#define irSens 4
#define fire   5
#define motorA 6
#define motorB 7
#define buzzer 8

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
SoftwareSerial gsm(3, 2);

int soil, event, oldVal;
bool state, smsFlag;
long prevMs;

void setup() {
  Serial.begin(9600);
  gsm.begin(9600);
  lcd.begin(16, 2);

  pinMode(irSens, INPUT);
  pinMode(fire, INPUT);
  pinMode(motorA, OUTPUT);
  pinMode(motorB, OUTPUT);
  pinMode(buzzer, OUTPUT);

  motorOff();
  GSMinit();
}

void loop() {
  soil = analogRead(A6);
  soil = map(soil, 0, 1023, 100, 0);

  if (soil < MOTOR_ON_AT) motorOn();
  if (soil > MOTOR_OFF_AT) motorOff();

  if (!digitalRead(fire)) event = 1;
  if (!digitalRead(irSens)) event = 2;
  if (digitalRead(fire) && digitalRead(irSens)) event = 0;

  if (oldVal != event) {
    oldVal = event;
    lcd.clear();
  }

  if (millis() - prevMs >= 500) {
    if (event == 0) {
      lcd.setCursor(0, 0);
      lcd.print((String) "SOIL LEVEL: " + soil + "%  ");
      lcd.setCursor(0, 1);
      lcd.print("MOTOR ");
      lcd.print(state ? "ON " : "OFF");
      lcd.print(", SAFE.");
      digitalWrite(buzzer, 0);
      smsFlag = 0;
    }
    else if (event == 1) {
      lcd.setCursor(0, 1);
      lcd.print("FIRE DETECTED!!!");
      digitalWrite(buzzer, !digitalRead(buzzer));
      if (smsFlag == 0) sendSMS(fireMsg);
    }
    else if (event == 2) {
      lcd.setCursor(0, 1);
      lcd.print("ANIMAL DETECTED!");
      digitalWrite(buzzer, !digitalRead(buzzer));
      if (smsFlag == 0) sendSMS(animalMsg);
    }

    prevMs = millis();
  }
}

void GSMinit() {
  lcd.print("Connecting GSM..");
  delay(5000);
  gsm.println("AT");
  delay(1000);
  gsm.println("ATE0");
  delay(1000);
  gsm.println("AT+CMGF=1");
  delay(1000);
  gsm.println("AT+CNMI=1,2,0,0,0");
  delay(1000);
  lcd.clear();
}

void sendSMS(const char *txt) {
  lcd.clear();
  lcd.print("Sending SMS...");
  gsm.print("AT+CMGF=1\r\n");
  delay(500);
  gsm.print("AT+CMGS=\"");
  delay(500);
  gsm.print(number);
  gsm.print("\"\r\n");
  delay(500);
  gsm.print(txt);
  gsm.write(0x1A);
  gsm.print("\r\n");
  delay(3500);
  lcd.clear();
  smsFlag = 1;
}

void motorOn() {
  state = 1;
  digitalWrite(motorA, 1);
  digitalWrite(motorB, 0);
}

void motorOff() {
  state = 0;
  digitalWrite(motorA, 0);
  digitalWrite(motorB, 0);
}
