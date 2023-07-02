#include <Debounce.h>
#include <TM1650.h>
#include <TM16xxDisplay.h>
#define BRIGHTNESS 3 //do sang Led 7 doan
#define DATA 8 //ket noi voi IC dieu khien Led 7 doan
#define CLK 7 //ket noi voi IC dieu khien Led 7 doan
#define TRIG 12 //nut nhan kich hoat han
#define OPTO 5 //chan ra opto MOC3041
#define GREENLED 11 //den Led bao trang thai (kich hoat muc thap)
#define BUZZ 6 // buzzer (kich hoat muc cao)
TM1650 fd650(DATA,CLK,4,true,BRIGHTNESS);
TM16xxDisplay disp(&fd650,4);
enum BUTTONS {NONE,UP,DOWN,SET,WELD};// cac trang thai nut bam
enum STATES {NORMAL,PULSES,PULSE1,PULSE2}; //cac trang thai hien thi
STATES runMode=NORMAL;
#define FLASH_TIME 200
#define STEP_TIME 350
#define SWAP_TIME 1500 //thoi gian chuyen qua lai giua hien thi delay time va so xung
long flashTimeout=0;
bool flashOn=false;
long stepTimeout=0;
int pulse1_delay=30; //xung thu 1
int pulse2_delay=85; //xung thu 2 (xung han cell)
int pulses=1; //xung don hoac kep
//bool updateTime=true;
long pastSeconds=0;
Button btn;

void beepbeep(int,int);

void setup() {
  // put your setup code here, to run once:
//Serial.begin(115200);
pinMode(TRIG,INPUT_PULLUP);
pinMode(OPTO,OUTPUT);
digitalWrite(OPTO,HIGH);
btn.begin(TRIG);
pinMode(GREENLED,OUTPUT);
pinMode(BUZZ,OUTPUT);
beepbeep(5,100);
}

void beepbeep(int numofbeeps,int duration){
  int state=HIGH;
    for(int i=0;i<numofbeeps;i++){
      digitalWrite(BUZZ,state);
      digitalWrite(GREENLED,!state);
      delay(duration);
      state=!state;
}
    digitalWrite(BUZZ,LOW);
    digitalWrite(GREENLED,HIGH);
}

BUTTONS getButtonState(){
  BUTTONS result=NONE;
  int val=analogRead(A0);
  //Serial.println(val);
  if(val<100){
    result=UP;
  }else if(val<550){
    result=DOWN;
  }else if(val<800){
    result=SET;
  }
  if(digitalRead(TRIG)==0){
    result=WELD;
  }
  return result;
}
void displayMode(){
  if(runMode==NORMAL){
    if(millis()>pastSeconds){
    pastSeconds=millis()+SWAP_TIME;
    long begin_time=millis();
    flashOn=!flashOn;
    if(flashOn){
      //disp.setDisplayToDecNumber(delayTime,0b0000);
      if(pulses==1){
        disp.setDisplayToDecNumber(pulse2_delay,0,true);
      }else{
        disp.setDisplayToDecNumber(pulse1_delay,0,true);
        while(millis()-begin_time<500){
       
          }
      //fd650.clearDisplay();
      disp.setDisplayToDecNumber(pulse2_delay,0,true);   
      }
     
      
    }else{
       //disp.setDisplayToDecNumber(pulses,0b0000);
       disp.setDisplayToDecNumber(pulses,0,true);
       
    }
  }
    
 // updateTime=false;
  }
}

void adjustPulse1Mode(){
  
  if(millis()>flashTimeout){
    flashTimeout=millis()+FLASH_TIME;
    flashOn=!flashOn;
    if(flashOn){
      //disp.setDisplayToDecNumber(delayTime,0b0000);
      disp.setDisplayToDecNumber(pulse1_delay,0,true);
    }else{
     fd650.clearDisplay();
    }
  }
  if(millis()>stepTimeout){
    if(getButtonState()==UP){
      pulse1_delay+=1;
      if(pulse1_delay>50) pulse1_delay=50;
    //disp.setDisplayToDecNumber(delayTime,0b0000);
    disp.setDisplayToDecNumber(pulse1_delay,0,true);
    stepTimeout=millis()+STEP_TIME;
    }else if(getButtonState()==DOWN){
      pulse1_delay-=1;
      if(pulse1_delay<=0) pulse1_delay=1;
      //disp.setDisplayToDecNumber(delayTime,0b0000);
      disp.setDisplayToDecNumber(pulse1_delay,0,true);
      stepTimeout=millis()+STEP_TIME;
    }
  }
}

void adjustPulse2Mode(){
  
  if(millis()>flashTimeout){
    flashTimeout=millis()+FLASH_TIME;
    flashOn=!flashOn;
    if(flashOn){
      //disp.setDisplayToDecNumber(delayTime,0b0000);
      disp.setDisplayToDecNumber(pulse2_delay,0,true);
    }else{
     fd650.clearDisplay();
    }
  }
  if(millis()>stepTimeout){
    if(getButtonState()==UP){
      pulse2_delay+=1;
      if(pulse2_delay>150) pulse2_delay=150;
    //disp.setDisplayToDecNumber(delayTime,0b0000);
    disp.setDisplayToDecNumber(pulse2_delay,0,true);
    stepTimeout=millis()+STEP_TIME;
    }else if(getButtonState()==DOWN){
      pulse2_delay-=1;
      if(pulse2_delay<=0) pulse2_delay=1;
      //disp.setDisplayToDecNumber(delayTime,0b0000);
      disp.setDisplayToDecNumber(pulse2_delay,0,true);
      stepTimeout=millis()+STEP_TIME;
    }
  }
}


void adjustPulsesMode(){
  
  if(millis()>flashTimeout){
    flashTimeout=millis()+FLASH_TIME;
    flashOn=!flashOn;
    if(flashOn){
      //disp.setDisplayToDecNumber(pulses,0b0000);
      disp.setDisplayToDecNumber(pulses,0,true);
    }else{
     fd650.clearDisplay();
    }
  }
  if(millis()>stepTimeout){
    if(getButtonState()==UP){
      pulses+=1;
      if(pulses>2) pulses=2;
    //disp.setDisplayToDecNumber(pulses,0b0000);
    disp.setDisplayToDecNumber(pulses,0,true);
    stepTimeout=millis()+STEP_TIME;
    }else if(getButtonState()==DOWN){
      pulses-=1;
      if(pulses<=1) pulses=1;
      //disp.setDisplayToDecNumber(pulses,0b0000);
      disp.setDisplayToDecNumber(pulses,0,true);
      stepTimeout=millis()+STEP_TIME;
    }
  }
}

bool testSetButton(){
  bool result=false;
 if(getButtonState()==SET){
  //delay(60);
  if(getButtonState()==SET){
    runMode=(runMode==PULSE2)?NORMAL:(STATES)((int)runMode+1);
    switch (runMode){
      case PULSES:
      flashTimeout=millis()+FLASH_TIME;
      flashOn=false;
     fd650.clearDisplay();
      break;
      case PULSE1:
      flashTimeout=millis()+FLASH_TIME;
      flashOn=false;
     fd650.clearDisplay();
     case PULSE2:
      flashTimeout=millis()+FLASH_TIME;
      flashOn=false;
     fd650.clearDisplay();
      case NORMAL:
      //disp.setDisplayToDecNumber(delayTime,0b0000);
      //disp.setDisplayToDecNumber(pulse1_delay,0,true);
      
      result=true;
    }
    while(getButtonState()){
      //delay(10);
    }
  }
 }
 return result; 
}

void spotWelder(int pulse1_delay_time, int pulse2_delay_time, int pulses_num=1){
  long begin_time=millis();
  bool timeout;
  if(btn.debounce()){
    //delay(10);
    
    switch(pulses_num){
        case 1:
        digitalWrite(OPTO,LOW);
        begin_time=millis();
        while(millis()-begin_time<pulse2_delay_time){
        timeout=false;
        }
        digitalWrite(OPTO,HIGH);           
        fd650.clearDisplay();
        disp.setDisplayToDecNumber(1111,0,true);
        beepbeep(1,100);
        break;
        case 2:
        digitalWrite(OPTO,LOW);
        begin_time=millis();
        while(millis()-begin_time<pulse1_delay_time){// xung thu 1
        timeout=false;
        }
        digitalWrite(OPTO,HIGH);
        begin_time=millis();
        while(millis()-begin_time<250){ //khoang nghi giua 2 xung
        timeout=false;
        }
        digitalWrite(OPTO,LOW);
        begin_time=millis();
        while(millis()-begin_time<pulse2_delay_time){ //xung thu 2
        timeout=false;
        }
        digitalWrite(OPTO,HIGH);
        fd650.clearDisplay();
        disp.setDisplayToDecNumber(2222,0,true);
        beepbeep(3,100);
        break;
      }
    
   }
 }
 
void loop() {
  testSetButton();//nut SET co duoc nhan hay khong
 
  // put your main code here, to run repeatedly:
  switch (runMode){
    case NORMAL:
    displayMode();
    spotWelder(pulse1_delay,pulse2_delay,pulses);
    break;
    case PULSES: adjustPulsesMode(); break;
    case PULSE1: adjustPulse1Mode(); break;
    case PULSE2: adjustPulse2Mode(); break;
  }
  
}
