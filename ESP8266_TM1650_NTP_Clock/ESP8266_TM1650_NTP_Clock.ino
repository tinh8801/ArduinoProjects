#include <ESP8266WebServer.h>
#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>
#include <TM1650.h>
#include <TM16xxDisplay.h>
#include <WiFiClient.h>
#include <DHT.h>
#define DHTPIN 14
#define DHTTYPE DHT11
#define DATA 4
#define CLK 5

DHT dht(DHTPIN,DHTTYPE);
TM1650 module(DATA,CLK,4,true,3,TM1650_DISPMODE_4x8);   // DIO=8, CLK=9, STB=7
#define NUM_DIGITS 4
TM16xxDisplay display(&module, NUM_DIGITS);    // TM16xx object, 8 digits
int timescroll=500;
int datescroll=500;
byte brightness=7;
bool isNight;
bool autobrightness=true;
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
p {
color: navy;
text-transform: uppercase;
}
</style>
</head>
<body>

<h2>ESP8266 NTP Clock<h2>
<h3>Settings</h3>

<form action="/action_page">
  Time Scroll Speed
  <input type="text" name="timescroll" value=""> (ms)
  <br><br>
  Date Scroll Speed
  <input type="text" name="datescroll" value=""> (ms)
  <br><br>
  <input type="submit" value="Set">
</form>
<br><br>
<form action="/brightness_page">
Display Brightness
  <input type="text" name="brightness" value=""> (0-7)
     <input type="submit" value="Set">
</form> 
<br><br>
<form action="/autobrightness_page">
Auto Brightness
  <input type="text" name="autobrightness" value=""> (yes/no)
     <input type="submit" value="Set">
</form> 
<br><br>
<font size='18' color='red'>Reset</font>
<form action="/reset_page">
     <input type="submit" value="Reset">
</form> 
@@@
</body>
</html>
)=====";

const char *ssid     = "DDWRT";
const char *password = "12347890";
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"asia.pool.ntp.org",7*3600,60000);
int prevmin=0;
String dt="";

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
int getTemp(){
  float t=dht.readTemperature();
  if(isnan(t)){
    return 0;
  }else{
    return (int)t;
  }
}

int getHumid(){
  float h=dht.readHumidity();
  if(isnan(h)){
    return 0;
  }else{
    return (int)h;
  }
}

void setup(){
  //Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
   // Serial.print ( "." );
    display.println("----");
  }
  
  //display.setCursor(2);
  timeClient.begin();
  //timeClient.setTimeOffset(7*3600);
  timeClient.update();
  prevmin=timeClient.getMinutes();
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

String getTime(){
String kq="";
String phut="";
  if(timeClient.getHours()>12){
    kq=String(timeClient.getHours()-12);
  }else{
    kq=String(timeClient.getHours());
  }
  phut=String(timeClient.getMinutes());
  if(phut.length()<2){
    return kq+"h0"+phut;
  }else{
    return kq+"h"+phut;  
  }   
}

int nPos=0;
//String szText="";
void scrollTime(String t, int spd){
  
  //Serial.println(szText);
  display.setCursor(nPos);
  display.println(t);
  nPos--;
  if(nPos == 0-t.length()-1)
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

String getDate(){
  String ngaythang="";
time_t epTime=timeClient.getEpochTime();
   struct tm *ptm=gmtime((time_t *)&epTime);
   int currDay=ptm->tm_mday;
   int currMonth=ptm->tm_mon+1;
   int currYear=ptm->tm_year+1900;
return ngaythang=String(currDay)+"-"+String(currMonth)+"-"+String(currYear);  
}

void loop() {
    server.handleClient();
    if(timeClient.getHours()==0){
      dt=getDate();
    }
    if(timeClient.getHours()<7){
      isNight=true;
    }else{
      isNight=false;
    }
    
    if(autobrightness){
      if(isNight){
        brightness=1;
      }else{
        brightness=5;
      }
    }else{
      //brightness=7;
    }
    display.setIntensity(brightness);
    
    if(timeClient.getMinutes()>prevmin){
      timeClient.update();
      prevmin=timeClient.getMinutes();
    }
      if(timeClient.getSeconds()<25){
        scrollTime(getTime(),timescroll);
      }else{
        if(timeClient.getSeconds()>25 && timeClient.getSeconds()<40){
          scrollDate(dt,datescroll);
          }
        if(timeClient.getSeconds()>40 && timeClient.getSeconds()<50){
          int nhietdo=getTemp();
          //String temp;
          //temp.concat(nhietdo);
          //display.clear();
           display.setDisplayToDecNumber(nhietdo,0,false);
        }
        if(timeClient.getSeconds()>50 && timeClient.getSeconds()<59){
          int doam=getHumid();
          //String temp1;
          //temp1.concat(doam);
          //display.clear();
          display.setDisplayToDecNumber(doam,0,false);  
        }
      }
   
}
