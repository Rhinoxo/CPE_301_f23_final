#define powerButton 2
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

#define tempSensor 0
#define waterSensor 5

#define waterLimit 10
#define tempLimit 150

#include <LiquidCrystal.h>

int waterLevel = 0;
int tempLevel = 0;

bool active = false;

enum states {IDLE, RUN, OFF, ERR};
int state = OFF;

LiquidCrystal lcd(lcdRS, lcdEN, lcdD4, lcdD5, lcdD6, lcdD7);

void setup(){
  Serial.begin(9600);

  pinMode(fanPIN, OUTPUT);

  pinMode(IDLELED, OUTPUT);
  pinMode(RUNLED, OUTPUT);
  pinMode(OFFLED, OUTPUT);
  pinMode(ERRLED, OUTPUT);
  
  lcd.begin(16,2);

  pinMode(powerButton, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(powerButton), powerToggle, RISING);
}

void loop(){
  delay(1000);
  lcd.clear();
  
  tempLevel = analogRead(tempSensor);
  waterLevel = analogRead(waterSensor);

  if(state == IDLE || state == RUN){
    if(waterLevel <= waterLimit){state = ERR;}
    else if(tempLevel > tempLimit){state = RUN;}
    else{state = IDLE;}
  }

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
  active = !active;

  if(active && state != RUN){state = IDLE;}
  else if(!active){state = OFF;}
  loop();
}
