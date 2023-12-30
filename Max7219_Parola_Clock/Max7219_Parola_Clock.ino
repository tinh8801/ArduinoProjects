#include <MD_Parola.h>
#include <MD_MAX72xx.h> 
#include <SPI.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//#include <WiFiClient.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 14
#define DATA_PIN 13
#define CS_PIN 15
#define SPEED_TIME 75
#define PAUSE_TIME 0
#define NUM_ZONE 2
MD_Parola disp = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

char zoneMin[4]="";
char zoneHour[4]="";

bool flasher=false;

const char *ssid     = "DDWRT";
const char *password = "12347890";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"asia.pool.ntp.org",7*3600,60000);
int lastMin=0;
int lastHour=0;
String dt="";

String getDateString(){
     String ngaythang="";
    time_t epTime=timeClient.getEpochTime();
    struct tm *ptm=gmtime((time_t *)&epTime);
    int currDay=ptm->tm_mday;
    int currMonth=ptm->tm_mon+1;
    int currYear=ptm->tm_year+1900;
    return ngaythang=String(currDay)+"-"+String(currMonth)+"-"+String(currYear);
}

void updateMin(char *p,bool f){
int kq=timeClient.getMinutes();
sprintf(p, "%c%02d", (f?':':' '), kq);
}

void updateHour(char *p){
int kq=timeClient.getHours();
sprintf(p, "%02d", kq);
}

void updateDate(char *p){
strcpy(p,getDateString().c_str());  
}

void setup(){
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print ( "." );
    //display.println("----");
    }
  //display.setCursor(2);
  timeClient.begin();
  timeClient.update();
  lastMin=timeClient.getMinutes();
  lastHour=timeClient.getHours();
  dt=getDateString();
  
  //Serial.println("Web server dang khoi dong. Vui long doi dia chi IP…");
  delay(1000);
  //Serial.println(WiFi.localIP());

  disp.begin(NUM_ZONE);
  disp.setInvert(false);
  disp.setIntensity(3);
  disp.setZone(0,0,1);
  disp.setZone(1,2,3);
  
  disp.displayZoneText(0,zoneMin,PA_CENTER,SPEED_TIME,PAUSE_TIME,PA_PRINT,PA_NO_EFFECT);
  disp.displayZoneText(1,zoneHour,PA_CENTER,SPEED_TIME,PAUSE_TIME,PA_PRINT,PA_NO_EFFECT);
  
  updateMin(zoneMin,flasher);
  updateHour(zoneHour);
}

void loop(){
  
  disp.displayAnimate();
  if(timeClient.getHours()==0){
      dt=getDateString();
    }
  if(timeClient.getMinutes()>lastMin){
      timeClient.update();
      lastMin=timeClient.getMinutes();
    }
    
    if(timeClient.getSeconds()%2){
      flasher=true;
    }else{
      flasher=false;
    }
    
    if(disp.getZoneStatus(0)){
      if(timeClient.getSeconds()>58){
        disp.setTextEffect(0, PA_SCROLL_UP, PA_WIPE);
        updateMin(zoneMin,false);
      }else{
        disp.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
        updateMin(zoneMin,flasher);
      }
       disp.displayReset(0);
    }
    
   delay(100);
   
    if(disp.getZoneStatus(1)){
      if((timeClient.getMinutes()==59)&&(timeClient.getSeconds()>58)){
        disp.setTextEffect(1,PA_SCROLL_DOWN, PA_WIPE);
        updateHour(zoneHour);
          }else{
            disp.setTextEffect(1,PA_PRINT, PA_NO_EFFECT);
            updateHour(zoneHour);
          }
  }
   disp.displayReset(1);
}
	
