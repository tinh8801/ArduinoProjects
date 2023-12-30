#define ledPin 2
#define BLINK 200
long batdau;
long hientai;
void (*runningFunc)();
bool ledState;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  runningFunc=&resetFunc;

}

void loop() {
  // put your main code here, to run repeatedly:
runningFunc();
}
void resetFunc(){
  batdau=millis();
  if(true) runningFunc=&checktimeFunc;
}
void checktimeFunc(){
  hientai=millis();
  if((hientai-batdau)>BLINK) runningFunc=&checkledstateFunc;
}
void checkledstateFunc(){
  ledState=digitalRead(ledPin);
  if(ledState==HIGH) runningFunc=&turnonledFunc;
  else runningFunc=&turnoffledFunc;
}
void turnonledFunc(){
  digitalWrite(ledPin, LOW);
  if(true) runningFunc=&resetFunc;
}
void turnoffledFunc(){
  digitalWrite(ledPin, HIGH);
  if(true) runningFunc=&resetFunc;
}
