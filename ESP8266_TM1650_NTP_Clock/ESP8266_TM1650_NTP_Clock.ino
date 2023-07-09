#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TM1650.h>
#include <TM16xxDisplay.h>
#include <DHT.h>
#include <WiFiManager.h>
#include "myweb.h"
#define DHTPIN 13
#define DHTTYPE DHT11
#define DATA 12
#define CLK 14
#define NUM_DIGITS 4

DHT dht(DHTPIN,DHTTYPE);
TM1650 module(DATA,CLK,4,true,4,TM1650_DISPMODE_4x8);   // DIO=8, CLK=9, STB=7
TM16xxDisplay display(&module, NUM_DIGITS);    // TM16xx object, 8 digits
int datescroll=500;
int brightness;
bool isNight;
bool autobrightness=true;
int * _temphumid;
char _temp[6]="";
char _humid[6]="";
char _heatindex[6]="";
bool Mode24h=true;
char _thoigian[6]="";
String dow[7]={"SUn","MOn","tUE","WEd","tHU","FrI","SAt"};
String _dow="";

//const char *ssid     = "DDWRT";
//const char *password = "12347890";

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"asia.pool.ntp.org",7*3600,60000);
String _datetime="";

String getDate(){
  String kq="";
  time_t epTime=timeClient.getEpochTime();
  struct tm *ptm=gmtime((time_t *)&epTime);
  int currDay=ptm->tm_mday;
  int currMonth=ptm->tm_mon+1;
  int currYear=ptm->tm_year+1900;
  return kq=String(currDay)+"-"+String(currMonth)+"-"+String(currYear);  
}

int * getTime(bool f=true){//true=24h false=12h
  static int kq[3];
    kq[0]=timeClient.getSeconds();
    kq[1]=timeClient.getMinutes();
    if(f){
    kq[2]=timeClient.getHours();
    }else{
      if(timeClient.getHours()>12){
        kq[2]=timeClient.getHours()-12;
      }else{
        kq[2]=timeClient.getHours();
      }
    }
    return kq;
}

int * getTempHumid(){
  static int kq[3];
  float t=dht.readTemperature();
  float h=dht.readHumidity();
  if(isnan(t) || isnan(h)){
    kq[0]=99;
    kq[1]=99;
    kq[2]=99;
    return kq;
  }else{
    kq[0]=(int)t;
    kq[1]=(int)h;
    kq[2]=(int)dht.computeHeatIndex(t,h,false);
    return kq;
  }
}


int nPos=0;
void scrollTime(String thoigian, int spd){
  display.setCursor(nPos);
  display.println(thoigian);
  nPos--;
  if(nPos == 0-thoigian.length()-1)
  {
    display.clear();
    nPos=NUM_DIGITS;
  }
  delay(spd);
}

int nPos1=0;
void scrollDate(String ngaythang,int spd){
  display.setCursor(nPos1);
  display.println(ngaythang);
  nPos1--;
  if(nPos1 == 0-ngaythang.length()-1)
  {
    display.clear();
    nPos1=NUM_DIGITS;
  }
  delay(spd);
}

//==============================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
//===============================================================

void handleReset(){
   datescroll=500;
   autobrightness=true;
  String s = "<a href='/'><font size='20'>Go Back</font></a>";
  server.send(200, "text/html", s); //Send web page
}

void handleUpdateStatus(){
  String stat="Date Scroll: ";
 stat.concat(datescroll);
 stat+="<br>Intensity: ";
 stat.concat(brightness);
 stat+="<br>Auto Brightness: ";
 if(autobrightness){
  stat+="ON";
 }else{
  stat+="OFF";
 }
 stat+="<br>Is Night: ";
 if(isNight){
  stat+="ON";
 }else{
  stat+="OFF";
 }
 stat+="<br>";
  server.send(200,"text/plane",stat);
  }

void handleScroll() {
  String DateSpeed = server.arg("scrollVal"); 
 datescroll = abs(DateSpeed.toInt())*100;
}

void handleIntensity(){
 String Brightness = server.arg("intensityVal");
  if(autobrightness==false){
  brightness=abs(Brightness.toInt());
  }
}

void handleAutoIntensity(){
 String AutoIntensity = server.arg("autoIntensity");
  if(AutoIntensity=="ON"){
    autobrightness=true;   
 }
 if(AutoIntensity=="OFF"){
  autobrightness=false; 
 }
}

void handleDisplayMode(){
 String Display12hMode = server.arg("display12hMode");
  if(Display12hMode=="ON"){
    Mode24h=false;   
 }
 if(Display12hMode=="OFF"){
  Mode24h=true; 
 }
}

void handleUpdate12h(){
String stat="";
if(Mode24h){
  stat="false";
}else{
  stat="true";
}
  server.send(200,"text/plane",stat);
}

void handleUpdateAI(){
String stat="";
if(autobrightness){
  stat="true";
}else{
  stat="false";
}
  server.send(200,"text/plane",stat);
}

void handleUpdateScroll(){
String stat=String(datescroll/100);
  server.send(200,"text/plane",stat);
}

void handleUpdateIntensity(){
String stat=String(brightness);
  server.send(200,"text/plane",stat);
}

//=================================================
void setup(){
  dht.begin();
 // WiFi.begin(ssid, password);
  //while(WiFi.status() != WL_CONNECTED) {
    //delay(500);
   // Serial.print ( "." );
    //display.println("----");
  //}
  WiFiManager wm;
  if(!wm.autoConnect("TM1650Clock")){
  
  }
  display.println("----");
  timeClient.begin();
  timeClient.update();
  _temphumid=getTempHumid();
  sprintf(_temp,"%c %02d",'T',_temphumid[0]);//(char(26) mean degree C
  sprintf(_humid,"%c %02d",'H',_temphumid[1]);
  sprintf(_heatindex,"%c%c%02d",'h','i',_temphumid[2]);
  _datetime=getDate();
  _dow=dow[timeClient.getDay()];
  server.on("/",handleRoot);      //Which routine to handle at root location
  server.on("/reset_page",handleReset);
  server.on("/setScroll",handleScroll); //form action is handled here
  server.on("/setIntensity",handleIntensity);
  server.on("/toggleAIMode",handleAutoIntensity);
  server.on("/toggleDisplayMode",handleDisplayMode);
  server.on("/updateStatus",handleUpdateStatus);
  server.on("/update12hCheckbox",handleUpdate12h);
  server.on("/updateAICheckbox",handleUpdateAI);
  server.on("/updateScrollSlider",handleUpdateScroll);
  server.on("/updateIntensitySlider",handleUpdateIntensity);
  server.begin();                  //Start server
  //Serial.println("Web server dang khoi dong. Vui long doi dia chi IP…");
  delay(1000);
  //Serial.println(WiFi.localIP());
  char IPAddr[(uint8_t)20];
    sprintf(IPAddr,"%d.%03d",WiFi.localIP()[2],WiFi.localIP()[3]);
    if(timeClient.isTimeSet()){
   display.println(IPAddr);
    }
    delay(2000);
}

void loop() {
  timeClient.update();
    server.handleClient();
    if(timeClient.getHours()==0){
      _datetime=getDate();
       _dow=dow[timeClient.getDay()];
    }

    int * t=getTime(Mode24h);
  //strcpy(thoigian,getTimeString().c_str());
    if(timeClient.getSeconds()%2){
    sprintf(_thoigian,"%02d.%02d",t[2],t[1]);
    }else{
    sprintf(_thoigian,"%02d%02d",t[2],t[1]);  
    }
   if((timeClient.getHours()>=0 && timeClient.getHours()<7)||(timeClient.getHours()>=22)){//auto set brightness to low in 22pm to 7am 
    isNight=true;
   }else{
    isNight=false;
      }
    
    if(autobrightness){
      if(isNight){
        brightness=1;
        display.setIntensity(brightness);
      }else{
        brightness=4;
        display.setIntensity(brightness);
      }
    }else{
      display.setIntensity(brightness);
    }

   if(timeClient.getMinutes()%2 && timeClient.getSeconds()%3){
        _temphumid=getTempHumid();
        sprintf(_temp,"%c %02d",'T',_temphumid[0]);//(char(26) mean degree C
        sprintf(_humid,"%c %02d",'H',_temphumid[1]);
        sprintf(_heatindex,"%c%c%02d",'h','i',_temphumid[2]);
    }
    
      if(timeClient.getSeconds()<20){
        //scrollTime(getTime(),timescroll);
        display.println(_thoigian);
      }else if(timeClient.getSeconds()<30){
          scrollDate(_datetime,datescroll);
      }else if(timeClient.getSeconds()<35){      
          display.println(_dow);
      }else if(timeClient.getSeconds()<45){
          display.println(_temp);  
        }else if(timeClient.getSeconds()<55){
          display.println(_humid);  
        }else{
          display.println(_heatindex);  
        }
      }
   
