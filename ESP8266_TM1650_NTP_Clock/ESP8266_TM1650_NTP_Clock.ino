#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TM1650.h>
#include <TM16xxDisplay.h>
#include <DHT.h>
#include <WiFiManager.h>
#include "myweb.h"
#define DHTPIN 14
#define DHTTYPE DHT11
#define DATA 4
#define CLK 5
#define NUM_DIGITS 4

DHT dht(DHTPIN,DHTTYPE);
TM1650 module(DATA,CLK,4,true,4,TM1650_DISPMODE_4x8);   // DIO=8, CLK=9, STB=7
TM16xxDisplay display(&module, NUM_DIGITS);    // TM16xx object, 8 digits
int timescroll=500;
int datescroll=500;
byte brightness=7;
bool isNight;
bool autobrightness=true;

//const char *ssid     = "DDWRT";
//const char *password = "12347890";

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"asia.pool.ntp.org",7*3600,60000);
String dt="";

String getDate(){
  String kq="";
  time_t epTime=timeClient.getEpochTime();
  struct tm *ptm=gmtime((time_t *)&epTime);
  int currDay=ptm->tm_mday;
  int currMonth=ptm->tm_mon+1;
  int currYear=ptm->tm_year+1900;
  return kq=String(currDay)+"-"+String(currMonth)+"-"+String(currYear);  
}

String getTime(){
String kq="";
String phut="";
  kq=String(timeClient.getHours());
  phut=String(timeClient.getMinutes());
  if(phut.length()<2){
    return kq+"h0"+phut;
  }else{
    return kq+"h"+phut;  
  }   
}

int getTemp(){
  float t=dht.readTemperature();
  if(isnan(t)){
    return 99;
  }else{
    return (int)t;
  }
}

int getHumid(){
  float h=dht.readHumidity();
  if(isnan(h)){
    return 99;
  }else{
    return (int)h;
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
 String stat="<br><p>Time Scroll: ";
 stat.concat(timescroll);
 stat+="<br>Date Scroll: ";
 stat.concat(datescroll);
 stat+="<br>Display Brightness: ";
 stat.concat(brightness);
 stat+="<br>Auto Brightness: ";
 if(autobrightness){
  stat+="yes";
 }else{
  stat+="no";
 }
 stat+="<br>Is Night: ";
 if(isNight){
  stat+="yes";
 }else{
  stat+="no";
 }
 stat+="<br></p>";
 //Serial.print(stat);
 s.replace("@@@",stat);
 server.send(200, "text/html", s); //Send web page
}
//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
 String TimeSpeed = server.arg("timescroll"); 
 String DateSpeed = server.arg("datescroll"); 
 if(TimeSpeed!=""){
 timescroll = TimeSpeed.toInt();
 if(timescroll>1000){
  timescroll = 1000;
 }
 if(timescroll<100){
 timescroll = 100;
 }
 }else{
  timescroll=500;
 }
 
 if(DateSpeed!=""){
 datescroll = DateSpeed.toInt();
 if(datescroll>1000){
  datescroll = 1000;
 }
 if(datescroll<100){
  datescroll = 100;
 }
 }else{
  datescroll = 500;
 }
 
 String s = "<a href='/'><font size='20'>Go Back</font></a>";
 server.send(200, "text/html", s); //Send web page
}

void handleBrightness(){
 String Brightness =server.arg("brightness");
 if(autobrightness==false){
 if(Brightness!=""){
  brightness=Brightness.toInt();
  if(brightness<0){
    brightness=0;
  }
  if(brightness>7){
    brightness=7;
  }
  
 }else{
  brightness=5;
 }
 }
  String s = "<a href='/'><font size='20'>Go Back</font></a>";
  server.send(200, "text/html", s); //Send web page
}

void handleAutobrightness(){
 String Autobrightness =server.arg("autobrightness");
 if(Autobrightness!=""){
    if(Autobrightness=="yes"){
    autobrightness=true;   
  }
  if(Autobrightness=="no"){
    autobrightness=false;  
  }
 }else{
  autobrightness=true; 
 }
  String s = "<a href='/'><font size='20'>Go Back</font></a>";
  server.send(200, "text/html", s); //Send web page
}

void handleReset(){
  timescroll=500;
  datescroll=500;
  //brightness=7;
  autobrightness=true;
  
  String s = "<a href='/'><font size='20'>Go Back</font></a>";
  server.send(200, "text/html", s); //Send web page
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
  
  timeClient.begin();
  timeClient.update();
  
  dt=getDate();
  server.on("/", handleRoot);      //Which routine to handle at root location
  server.on("/action_page", handleForm); //form action is handled here
  server.on("/reset_page", handleReset);
  server.on("/brightness_page", handleBrightness);
  server.on("/autobrightness_page", handleAutobrightness);
  server.begin();                  //Start server
  //Serial.println("Web server dang khoi dong. Vui long doi dia chi IP…");
  delay(1000);
  //Serial.println(WiFi.localIP());
}

void loop() {
  timeClient.update();
    server.handleClient();
    if(timeClient.getHours()==0){
      dt=getDate();
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
    
      if(timeClient.getSeconds()<25){
        scrollTime(getTime(),timescroll);
      }else if(timeClient.getSeconds()<40){
          scrollDate(dt,datescroll);
      }else if(timeClient.getSeconds()<50){
          int nhietdo=getTemp();
          display.setDisplayToDecNumber(nhietdo,0,false);
      }else{
          int doam=getHumid();
          display.setDisplayToDecNumber(doam,0,false);  
        }
      }
   
