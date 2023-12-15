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

#define RDA 0x80
#define TBE 0x20

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

  U0init(9600);

  //pinMode(fanPIN, OUTPUT);
  DDRH |= 0x40;

  //pinMode(IDLELED, OUTPUT);
  //pinMode(RUNLED, OUTPUT);
  //pinMode(OFFLED, OUTPUT);
  //pinMode(ERRLED, OUTPUT);
  DDRB |= 0xF0;
  
  lcd.begin(16,2);

  //pinMode(powerButton, INPUT);
  //pinMode(resetButton, INPUT);
  DDRE &= ~(0x30);

  attachInterrupt(digitalPinToInterrupt(powerButton), powerToggle, RISING);
  attachInterrupt(digitalPinToInterrupt(resetButton), resetPress, RISING);

  adc_init();
}

void writeBit(volatile unsigned char*reg, unsigned char bit, bool state){
  bit = 0x01 << bit;
  if(state){*reg |= bit;}
  else{*reg &= ~bit;}
}

void loop(){
  sleep(1000);

  if(state == IDLE || state == RUN){
    tempLevel = dht.readTemperature(false, true);
    waterLevel = adc_read(waterSensor);

    if(waterLevel <= waterLimit){state = ERR;}
    else if(tempLevel > tempLimit){state = RUN;}
    else{state = IDLE;}
  }

  if(lastState != state){
    lastState = state;
    DateTime now = rtc.now();

    U0putchar("[");
    U0putchar(String(now.hour()));
    U0putchar(":");
    U0putchar(String(now.minute()));
    U0putchar(":");
    U0putchar(String(now.second()));
    U0putchar("] ");
  
    U0putchar(stateNames[state]);
    U0putchar("\n");
  }

  lcd.clear();
  lcd.setCursor(0,0);

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

  //digitalWrite(IDLELED, IDLELEDstate);
  writeBit(PORTB, 4, IDLELEDstate);
  //digitalWrite(RUNLED, RUNLEDstate);
  writeBit(PORTB, 3, RUNLEDstate);
  //digitalWrite(OFFLED, OFFLEDstate);
  writeBit(PORTB, 2, OFFLEDstate);
  //digitalWrite(ERRLED, ERRLEDstate);
  writeBit(PORTB, 1, ERRLEDstate);
  //digitalWrite(fanPIN, fanPINstate);
  writeBit(PORTH, 6, fanPINstate);
}

void powerToggle(){
  U0putchar("POWER\n");
  state = (state == OFF) ? IDLE : OFF;
}

void resetPress(){
  U0putchar("RESET\n");
  if(state == ERR){state = IDLE;}
}

void adc_init()
{
  // setup the A register
  ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode
  ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt
  ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
  // setup the B register
  ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    ADCSRB |= 0b00001000;
  }
}

void U0init(unsigned long U0baud)
{
 unsigned int tbaud;
 tbaud = (F_CPU / 16 / U0baud - 1);
 UCSR0A = 0x20;
 UCSR0B = 0x18;
 UCSR0C = 0x06;
 UBRR0  = tbaud;
}

unsigned char U0kbhit()
{
  return UCSR0A & RDA;
}

void U0putchar(unsigned char U0pdata)
{
  while((UCSR0A & TBE) == 0){};
  UDR0 = U0pdata;  
}

void U0putchar(String U0pdata){
  for(int i = 0; i < U0pdata.length(); i++){
    U0putchar(U0pdata[i]);
  }
}

void sleep(double period) {
  //Timer code
  double clkPeriod = 0.0000000625; // 1/16MHz
  // calc ticks
  unsigned int ticks = period / 2 / clkPeriod;
  // stop the timer
  TCCR1B &= 0xF8;
  // set the counts
  TCNT1 = (unsigned int) (65536 - ticks);
  // start the timer
  TCCR1A = 0x0;
  TCCR1B |= 0b00000001;
  // wait for overflow
  while((TIFR1 & 0x01)==0); // 0b 0000 0000
  // stop the timer
  TCCR1B &= 0xF8;   // 0b 0000 0000
  // reset TOV           
  TIFR1 |= 0x01;
}