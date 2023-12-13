#define powerButton 2
#define fanPIN 9

#define IDLELED 10
#define RUNLED 11
#define OFFLED 12
#define ERRLED 13

#define tempSensor 0

#define waterLimit 10
#define tempLimit 10

int waterLevel = 0;
int tempLevel = 0;

bool active = false;

enum states {IDLE, RUN, OFF, ERR};
int state = OFF;

void setup(){
  Serial.begin(9600);

  pinMode(fanPIN, OUTPUT);

  pinMode(IDLELED, OUTPUT);
  pinMode(RUNLED, OUTPUT);
  pinMode(OFFLED, OUTPUT);
  pinMode(ERRLED, OUTPUT);

  pinMode(powerButton, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(powerButton), powerToggle, RISING);
}

void loop(){
  delay(10);

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
      break;
    case RUN:
      RUNLEDstate = HIGH;
      fanPINstate = HIGH;
      break;
    case OFF:
      OFFLEDstate = HIGH;
      break;
    case ERR:
      ERRLEDstate = HIGH;
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
}
