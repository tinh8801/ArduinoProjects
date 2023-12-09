#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>
#include <TM1650.h>
#include <TM16xxDisplay.h>
#define DATA 4
#define CLK 5

TM1650 module(DATA,CLK,4,true,3,TM1650_DISPMODE_4x8);   // DIO=8, CLK=9, STB=7
#define NUM_DIGITS 4
TM16xxDisplay display(&module, NUM_DIGITS);    // TM16xx object, 8 digits


const char *ssid     = "TP-LINK_9B3852";
const char *password = "91890429";
WiFiServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"asia.pool.ntp.org",7*3600,60000);
int prevmin=0;
String dt="";

void setup(){
  //Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print ( "." );
    display.println("----");
  }
  //display.setCursor(2);
  timeClient.begin();
  //timeClient.setTimeOffset(7*3600);
  timeClient.update();
  prevmin=timeClient.getMinutes();
  dt=getDate();
  server.begin();
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
    if(timeClient.getHours()==0){
      dt=getDate();
    }
    if(timeClient.getMinutes()>prevmin){
      timeClient.update();
      prevmin=timeClient.getMinutes();
    }
      if((timeClient.getMinutes()%2==0)&(timeClient.getSeconds()<15)){
        scrollDate(dt,500);
      }else{
        scrollTime(getTime(),600);
      }
   
  WiFiClient client = server.accept();
  if (client)
  {
    //Serial.println("Co client moi");
    boolean blank_line = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
       // Serial.print(c);
        if (c == '\n' && blank_line)
        {      
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html><body>");
          client.println("<h1>ESP8266 NTP Clock with TM1650</h1>");
          client.println("<h2>TIME</h2><h3>Date: ");
          client.println(dt);
          client.println("</h3><h3>Time: ");
          client.println(getTime());
          client.println("</h3><h3>IP: ");
          client.println(WiFi.localIP());
          client.println("</h3><h3>");
          client.println("<h2>Code by Huy Thang</h2>");
          client.println("</body></html>");

          break;
        }
        if (c == '\r')
        {
          blank_line = true;
        }
        else if (c != '\r')
        {
          blank_line = false;
        }
      }
    }
    while (client.available()){
      char c=client.read();
      //Serial.print(c);
    }
    delay(1);
    client.stop();
    
    //Serial.println("Ngat ket noi client.");
  }
}
