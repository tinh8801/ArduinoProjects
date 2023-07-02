#include "myfont.h"
#include <WiFiManager.h>
#include <MD_MAX72xx.h> 
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>
#define DHTPIN 12 // DHT data pin
#define DHTTYPE DHT11 //DHT sensor type
//#include <WiFiClient.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW //Matrix board type
#define MAX_DEVICES 4 //Numbers of cascaded Matrix
#define CLK_PIN 14 //Esp8266 CLK pin
#define DATA_PIN 13 //Esp8266 MOSI pin
#define CS_PIN 15 //Esp8266 CS pin
#define CHAR_SPACING  1 // pixels between characters

DHT dht(DHTPIN,DHTTYPE);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

//const char *ssid     = "DDWRT";//Wifi SSID
//const char *password = "12347890";//Wifi Pass

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"asia.pool.ntp.org",7*3600,60000);
//int lastMin=0;
int * _date;
int * _temphumid;
String dow[7]={"SUN","MON","TUE","WED","THU","FRI","SAT"};
char _temp[8]="";
char _humid[8]="";
char _heatindex[8]="";
char dayofweek[4]="";
MD_MAX72XX::fontType_t *pFont=myfont;//use custom font in myfont.h

//============================================================

int * getDate(){//convert epoch time to date
    static int kq[4];
    time_t epTime=timeClient.getEpochTime();
    struct tm *ptm=gmtime((time_t *)&epTime);
    kq[0]=ptm->tm_mday;
    kq[1]=ptm->tm_mon+1;//tm_mon=0-11
    kq[2]=ptm->tm_year+1900;
    kq[3]=ptm->tm_wday;//day of week=0-6 (0=Sunday)
    return kq;

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

//=====================================================

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg){
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;//tong so cot 0-31
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  
  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  do{
    switch(state){
      case 0:
        if (*pMsg == '\0')
       {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
         state = 2;
          break;
        }
        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1:
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;
      case 2:
        curLen = 0;
        state++;
        // fall through

      case 3:
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;
      default:
        col = -1;   // this definitely ends the do loop
    }
  }while(col >= (modStart*COL_SIZE));
  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

//========================================================

void scrollText(const char *p,int delaytime)
{
  uint8_t charWidth;
  uint8_t cBuf[8];  // this should be ok for all built-in fonts

  mx.clear();

  while (*p != '\0')
  {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);

    for (uint8_t i=0; i<=charWidth; i++)  // allow space between characters
    {
      mx.transform(MD_MAX72XX::TSL);
      mx.setColumn(0, (i < charWidth) ? cBuf[i] : 0); 
      delay(delaytime);
    }
  }
}

//========================================================

void bounce()
// Animation of a bouncing ball
{
  const int minC = 0;
  const int maxC = mx.getColumnCount()-1;
  const int minR = 0;
  const int maxR = ROW_SIZE-1;

  int  nCounter = 0;

  int  r = 0, c = 2;
  int8_t dR = 1, dC = 1;  // delta row and column

  mx.clear();

  while (nCounter++ < 200)
  {
    mx.setPoint(r, c, false);
    r += dR;
    c += dC;
    mx.setPoint(r, c, true);
    delay(50);

    if ((r == minR) || (r == maxR))
      dR = -dR;
    if ((c == minC) || (c == maxC))
      dC = -dC;
  }
}

//========================================================

int * getTempHumid(){
  static int kq[3];
  float t=dht.readTemperature();
  float h=dht.readHumidity();

if(isnan(t) || isnan(h)){
    kq[0]=-99;
    kq[1]=-99;
    kq[2]=-99;
    return kq;
  }else{
    kq[0]=(int)t;
    kq[1]=(int)h;
    kq[2]=(int)dht.computeHeatIndex(t,h,false);
    return kq;
  }
}

//=======================================================

void transformation(char *p,bool z1=true,bool z2=false)//function to scroll up minute

{
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear(0,1);
  mx.clear(2,3);
    mx.setChar((COL_SIZE),p[4]);
    mx.setChar((2*COL_SIZE)-1,p[3]);
    //mx.setChar(16,p[2]);
    mx.setChar((3*COL_SIZE),p[1]);
    mx.setChar((4*COL_SIZE)-1,p[0]);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  delay(100);

  // run through the transformations
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);
    if(z1){
    for(int i=0;i<8;i++){
	mx.transform(0,1,MD_MAX72XX::TSD);
	delay(125);
  }
    }
    if(z2){
    for(int i=0;i<8;i++){
  mx.transform(2,3,MD_MAX72XX::TSU);
  delay(125);
  }
    }
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::OFF);
 mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

}
//============================

void setup(){
dht.begin();
mx.begin();
mx.setFont(pFont);//set custom font
  //WiFi.begin(ssid, password);
  //while(WiFi.status() != WL_CONNECTED) {
    //delay(500);
    //Serial.print ( "." );
    //bounce();//connecting wifi
   // }
  WiFiManager wm;
	if(!wm.autoConnect("Esp8266Clock")){
	//bounce();
	}
  timeClient.begin();
  bounce();
  delay(100);
  timeClient.update();
  //lastMin=timeClient.getMinutes();
  _date=getDate();
  _temphumid=getTempHumid();
 sprintf(_temp,"%c:%02d%c",'T',_temphumid[0],char(26));
  sprintf(_humid,"%c:%02d%c",'H',_temphumid[1],'%');
sprintf(_heatindex,"%c%c:%02d",'H','I',_temphumid[2]);  
sprintf(dayofweek,"%s",dow[_date[3]]);
  //Serial.println("Web server dang khoi dong. Vui long doi dia chi IP…");
  delay(1000);
  //Serial.println(WiFi.localIP());
  char IPAddr[(uint8_t)20];
    sprintf(IPAddr,"%03d:%03d:%03d:%03d",WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);
    if(timeClient.isTimeSet()){
    scrollText(IPAddr,100);
    }
}

void loop(){
  timeClient.update();
  char thoigian[8]="";
  char ngaythang[8]="";
  //lastMin=timeClient.getMinutes();
  int * t=getTime();
  //strcpy(thoigian,getTimeString().c_str());
  sprintf(thoigian,"%02d%c%02d",t[2],(timeClient.getSeconds()%2?':':' '),t[1]);
  sprintf(ngaythang,"%02d/%02d",_date[0],_date[1]);
  
  if(timeClient.getHours()==0){ //update date in 00:00
      _date=getDate();
      sprintf(dayofweek,"%s",dow[_date[3]]);
    }
  if(timeClient.getMinutes()%2 && timeClient.getSeconds()%3){
	_temphumid=getTempHumid();
  sprintf(_temp,"%c:%02d%c",'T',_temphumid[0],char(26));//(char(26) mean degree C
  sprintf(_humid,"%c:%02d %c",'H',_temphumid[1],'%');
  sprintf(_heatindex,"%c%c:%02d",'H','I',_temphumid[2]);
    }
  if((timeClient.getHours()>=0 && timeClient.getHours()<7)||(timeClient.getHours()>=22)){//auto set brightness to low in 22pm to 7am 
    mx.control(MD_MAX72XX::INTENSITY, 1);
  }else{
    mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY/2);
  }
  
    if(timeClient.getSeconds()<20){
	if(timeClient.getSeconds()==0){
	transformation(thoigian);	
	}else{    
      printText(0, MAX_DEVICES-1,thoigian);
	}
    }else if(timeClient.getSeconds()<45){
      if(timeClient.getSeconds()<30){
      printText(0, MAX_DEVICES-1,_temp);
	}else if(timeClient.getSeconds()<40){
      printText(0, MAX_DEVICES-1,_humid);
	}else{
      printText(0,MAX_DEVICES-1,_heatindex);
	}
    }else if(timeClient.getSeconds()<55){
      printText(0, MAX_DEVICES-1,ngaythang);
    }else{
      printText(0, MAX_DEVICES-1,dayofweek);
      if(timeClient.getSeconds()==59){
     // transformation(thoigian); 
        }
    }
}
	
