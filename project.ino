#define powerButton 18
#define resetButton 19
#define fanPIN 9

#define IDLELED 10
#define RUNLED 11
#define OFFLED 12
#define ERRLED 13

#define lcdRS 8
#define lcdEN 7
#define lcdD4 6
#define lcdD5 5
#define lcdD6 4
#define lcdD7 3

#define tempSensor 22
#define waterSensor 5

#define waterLimit 10
#define tempLimit 21

#include <LiquidCrystal.h>
#include <DHT.h>
#include <RTClib.h>

int waterLevel = 0;
float tempLevel = 0;

enum states {IDLE, RUN, OFF, ERR, SETUP};
String stateNames[4];

int lastState = SETUP;
int state = OFF;

LiquidCrystal lcd(lcdRS, lcdEN, lcdD4, lcdD5, lcdD6, lcdD7);
DHT dht(tempSensor, DHT11);
RTC_DS1307 rtc;

void setup(){
  stateNames[IDLE] = "IDLE";
  stateNames[RUN] = "RUNNING";
  stateNames[OFF] = "DISABLED";
  stateNames[ERR] = "ERROR";

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.begin(9600);

  pinMode(fanPIN, OUTPUT);

  pinMode(IDLELED, OUTPUT);
  pinMode(RUNLED, OUTPUT);
  pinMode(OFFLED, OUTPUT);
  pinMode(ERRLED, OUTPUT);
  
  lcd.begin(16,2);

  pinMode(powerButton, INPUT);
  pinMode(resetButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(powerButton), powerToggle, RISING);
  attachInterrupt(digitalPinToInterrupt(resetButton), resetPress, RISING);
}

void loop(){
  delay(1000);

  if(state == IDLE || state == RUN){
    tempLevel = dht.readTemperature(false, true);
    waterLevel = analogRead(waterSensor);

    if(waterLevel <= waterLimit){state = ERR;}
    else if(tempLevel > tempLimit){state = RUN;}
    else{state = IDLE;}
  }

  if(lastState != state){
    lastState = state;
    DateTime now = rtc.now();

    Serial.print("[");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.print(now.second());
    Serial.print("] ");
  
    Serial.println(stateNames[state]);
  }
  else{return;}

  lcd.clear();

  bool IDLELEDstate = LOW;
  bool RUNLEDstate = LOW;
  bool OFFLEDstate = LOW;
  bool ERRLEDstate = LOW;
  bool fanPINstate = LOW;

  switch(state){
    case IDLE:
      IDLELEDstate = HIGH;
      lcd.setCursor(0,1);
      lcd.print(tempLevel);
      break;
    case RUN:
      RUNLEDstate = HIGH;
      fanPINstate = HIGH;
      lcd.print("Running");
      lcd.setCursor(0,1);
      lcd.print(tempLevel);
      break;
    case OFF:
      OFFLEDstate = HIGH;
      break;
    case ERR:
      ERRLEDstate = HIGH;
      lcd.print("ERROR!");
      lcd.setCursor(0,1);
      lcd.print("Water too low!");
      break;
  }

  digitalWrite(IDLELED, IDLELEDstate);
  digitalWrite(RUNLED, RUNLEDstate);
  digitalWrite(OFFLED, OFFLEDstate);
  digitalWrite(ERRLED, ERRLEDstate);
  digitalWrite(fanPIN, fanPINstate);
}

void powerToggle(){
  Serial.println("POWER");
  state = (state == OFF) ? IDLE : OFF;
  loop();
}

void resetPress(){
  Serial.println("RESET");
  state = IDLE;
  loop();
}
