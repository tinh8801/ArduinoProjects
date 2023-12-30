#define ledPin 2
#define RESET 0
#define CHECKTIME 1
#define CHECKLEDSTATE 2
#define LEDON 3
#define LEDOFF 4
#define BLINK 500
long batdau;
long hientai;
byte state;
bool ledState;
void runningFunc();

void setup() {
  // put your setup code here, to run once:
pinMode(ledPin, OUTPUT);
digitalWrite(ledPin, HIGH);
state=RESET;
}

void loop() {
  runningFunc();
}

void runningFunc(){

  switch (state){
    case RESET:
      batdau=millis();
      if(true) state=CHECKTIME;
      break;
    case CHECKTIME:
      hientai=millis();
      if((hientai-batdau)>BLINK){
        state=CHECKLEDSTATE;
        }
      break;
    case CHECKLEDSTATE:
      ledState=digitalRead(ledPin);
      if(ledState==HIGH) state=LEDON;
      else state=LEDOFF;
      break;
    case LEDON:
      digitalWrite(ledPin,LOW);
      if(true) state=RESET;
      break;
    case LEDOFF:
      digitalWrite(ledPin,HIGH);
      if(true) state=RESET;
      break;
    
  }
}
