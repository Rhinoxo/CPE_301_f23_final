#define activePIN 2
#define fanPIN 10
#define powerLED 8
#define tempSensor 0

#define rangeHigh 100
#define rangeLow 25

bool activeButton = false;
bool isActive = false;
int angle = 10;

void readActive(){
  bool state = digitalRead(activePIN);
  if(state == activeButton){return;}
  activeButton = state;

  if(!activeButton){return;}

  isActive = !isActive;

  Serial.print("Active: ");
  Serial.println(isActive);
}

void setup(){
  Serial.begin(9600);

  pinMode(activePIN, INPUT);
  pinMode(fanPIN, OUTPUT);
  pinMode(powerLED, OUTPUT);
}

void loop(){
  delay(1);
  
  readActive();

  digitalWrite(powerLED, isActive);
  digitalWrite(fanPIN, isActive);

  if(!isActive){
    return;
  }

  int temp = analogRead(tempSensor);
}