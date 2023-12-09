#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_74HC595.h>
#define DS 14
#define SHCP 12
#define STCP 13
#define RS 7
#define E 6
#define D4 4
#define D5 3
#define D6 2
#define D7 1
#define BACKLIGHT 15
LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7, BACKLIGHT);

#define btnNext 4
#define btnPlay 5

enum BUTTONS {NONE,NEXT,PLAY};
#define lcdCols 16
#define lcdRows 2

const char *ssid     = "DDWRT";
const char *password = "12347890";
IPAddress host(192,168,2,8); // ip cua volumio
WiFiClient client;

long lastMillis = 0;
int interval = 0;
int retries=0;

BUTTONS checkButton(){
  BUTTONS kq=NONE;
  long baygio=millis();
  int count=0;
  if(digitalRead(btnNext)==LOW){//kiem tra trang thai nut nhan trong 100ms
    while(millis()-baygio<100){
      if(digitalRead(btnNext)==LOW){
        count++;
      }
    }
    if(count>50){
      kq=NEXT;//neu trang thai LOW keo dai du lau -> nut da thuc su duoc nhan
    }else{
      kq=NONE;
    }
  }
  if(digitalRead(btnPlay)==LOW){
    while(millis()-baygio<100){
      if(digitalRead(btnPlay)==LOW){
        count++;
      }
    }
    if(count>50){
      kq=PLAY;
    }else{
      kq=NONE;
    }
  }
    return kq;
}
/*
{"status":"play","position":5,"title":"Matsuri","artist":"Kitaro","album":"","albumart":"/albumart?cacheid=853&path=%2FUSB%2FMUSIC%2FLossless&metadata=false","uri":"mnt/USB/MUSIC/Lossless/Matsuri.ape","trackType":"ape","seek":87239,"duration":543,"samplerate":"44.1 KHz","bitdepth":"16 bit","channels":2,"random":false,"repeat":true,"repeatSingle":false,"consume":false,"volume":50,"disableVolumeControl":false,"mute":false,"stream":"ape","updatedb":false,"volatile":false,"service":"mpd"} 
 */

int getItem(String line, String item, char * value, int len) {//phan tich chuoi tra ve tu getState
  int pos1,pos2,pos3;
  //Serial.println("item=[" + String(item) + "]");
  pos1=line.indexOf(item);//vi tri cua chuoi status, title, artist
  //Serial.println("pos1=" + String(pos1));
  String line2;
  line2 = line.substring(pos1);
  //Serial.println("line2=[" + line2 + "]");
  pos2=line2.indexOf(":");//tim vi tri dau :
  pos3=line2.indexOf("\",");//tim vi tri ket thuc chuoi
 
  //Serial.println("pos2=" + String(pos2));
  //Serial.println("pos3=" + String(pos3));
  String line3;
  String temp;
  temp = line2.substring(pos2+2,pos3);//cat chuoi sau dau :
  if(temp.indexOf("(")>0){
  line3=temp.substring(0,temp.indexOf("(")-1);//cat bo phan nam trong dau ()
  }else{
    line3=temp;
  }
  string2char(line3, value, len);
  //Serial.println("value=[" + String(value) + "]");
  return(strlen(value));
}
void string2char(String line, char * cstr4, int len) {
  char cstr3[40];
  line.toCharArray(cstr3, line.length()+1);
  //Serial.println("cstr3=[" + String(cstr3) + "]");
  int pos4 = 0;
  for (int i=0;i<strlen(cstr3);i++) {
    //if (cstr3[i] == ' ') continue;
    if (cstr3[i] == ' ' && pos4 == 0) continue;
    cstr4[pos4++] = cstr3[i];
    cstr4[pos4] = 0;
    if (pos4 == (len-1)) break;
  }
  //Serial.println("cstr4=[" + String(cstr4) + "]");
}

void fillBuffer(char * line, int len){//neu line<16 ky tu thi bo sung bang ky tu 0x20
  int sz = strlen(line);
  for (int i=sz;i<len;i++) {
    line[i] = 0x20;
    line[i+1] = 0;
  }
}
void lcdDisplay(char * lcdbuf, int rows) {//xuat thong tin ra LCD
  char line[17];
  memset(line, 0, sizeof(line));
  strncpy(line, lcdbuf, 16);
  fillBuffer(line, 16);
  //Serial.println("line1=[" + String(line) + "]");
  lcd.setCursor(0, 0);
  lcd.print(line);//xuat ra hang 1 cua lcd 16 ky tu dau cua lcdbuf

  if (strlen(lcdbuf) > 16) {//neu lcdbuf>16 ky tu thi xuat ra dong thu 2
    strncpy(line, &lcdbuf[16], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  //Serial.println("line2=[" + String(line) + "]");
  lcd.setCursor(0, 1);
  lcd.print(line);

  if (rows == 2) return;//neu lcd loai 16x2 thi dung lai
  if (strlen(lcdbuf) > 32) {//neu lcd 16x4, 20x4... thi tiep tuc
    strncpy(line, &lcdbuf[32], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  //Serial.println("line3=[" + String(line) + "]");
  lcd.setCursor(0, 2);
  lcd.print(line);

  if (strlen(lcdbuf) > 48) {
    strncpy(line, &lcdbuf[48], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  //Serial.println("line4=[" + String(line) + "]");
  lcd.setCursor(0, 3);
  lcd.print(line);
}

//===================================================

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
    // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.println();
  //Serial.print("Wait for WiFi...");
  WiFi.begin(ssid, password);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
    cnt++;
    if ((cnt % 60) == 0){
      //Serial.println();
    }
  }
  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  lcd.begin(lcdCols, lcdRows);//khoi tao lcd
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Volumio Display");
  lcd.setCursor(0,1);
  lcd.print("WiFi Connected");
  delay(2000);
  lcd.noBacklight();
  pinMode(btnNext,INPUT_PULLUP);
  pinMode(btnPlay,INPUT_PULLUP);
  
}
//=======================================================
void loop() {
  // put your main code here, to run repeatedly:
  static int counter = 0;
  char state[40];//trang thai volumio (play, pause, stop)
  char lcdbuf[80] = {0};//noi dung se xuat ra lcd
  static char oldbuf[80] = {0};
  String payload="";
  
if(checkButton()==NEXT){
    HTTPClient http;
      http.begin(client,"http://192.168.2.8/api/v1/commands?cmd=next");
      int httpCode=http.GET();
      if(httpCode>0){
        //Serial.println("Button NEXT Pressed");
        }          
  }
  if(checkButton()==PLAY){
        HTTPClient http;
      http.begin(client,"http://192.168.2.8/api/v1/commands?cmd=toggle");
      int httpCode=http.GET();
      if(httpCode>0){
        //Serial.println("Button PLAY/PAUSE Pressed");
        }
  }
       
  long now = millis();
  if (now < lastMillis) lastMillis = now; // millis is overflow
  if (now - lastMillis > 1000) {
    lastMillis = now;
    counter++;
     
    HTTPClient http;
      http.begin(client,"http://192.168.2.8/api/v1/getState");
      int httpCode=http.GET();
      if(httpCode>0){//neu ket noi duoc voi volumio
       retries=0;
	 payload=http.getString();//kiem tra trang thai phat nhac
        // Serial.println(payload);
        int temp=getItem(payload, "status", state, sizeof(state));
        //Serial.println("state=" + String(state));
        } else{//khong ket noi duoc thi xoa man hinh va reset esp8266
          lcd.clear();
          lcd.noBacklight();
          delay(10*1000);
	retries+=1;
	if(retries>20){
          ESP.restart();
		} 
        }    
       
    if (counter > interval) {
    
 if(strcmp(state,"play") == 0) {
        char artist[40];
        char title[40];
        //char artisttrim[40];
        //char titletrim[40];
        int artistLen;
        int titleLen;
        artistLen = getItem(payload, "artist", artist, sizeof(artist));
        //Serial.println("Artist=" + String(artist));
        if((sizeof(artist)/sizeof(artist[0]))<1){
          strcpy(artist,"-----------");
        }
        titleLen = getItem(payload, "title", title, sizeof(title));
        //Serial.println("Title=" + String(title));
       /* int t=0;
        int a=0;
        for(int i=0;i<sizeof(title)-1;i++){
          if(title[i]=='('){
            t=i-1;
            break;
          }
        }
        for(int i=0;i<sizeof(artist)-1;i++){
          if(artist[i]=='('){
            a=i-1;
            break;
          }
        }
        
        memset(titletrim, 0, sizeof(titletrim));
        if(t==0){
            strcpy(titletrim,title);
        }else{
          strncpy(titletrim,title,t);
        }
        
        memset(artisttrim, 0, sizeof(artisttrim));
        if(a==0){
            strcpy(artisttrim,artist);
        }else{
          strncpy(artisttrim,artist,a);
        } */
        memset(lcdbuf, 0, sizeof(lcdbuf));
        if (artistLen > 0 && titleLen > 0) {
          strcpy(lcdbuf, title);
          strcat(lcdbuf, "#");
          strcat(lcdbuf, artist);
        } else if (artistLen == 0 && titleLen > 0) {
          strcpy(lcdbuf, title);
          strcat(lcdbuf,"#No Artist");
        }else{
          strcpy(lcdbuf, "No Info Song");
        }

        //Serial.println("lcdbuf=[" + String(lcdbuf) + "]");
        if (strlen(lcdbuf) > 0) {
          if (strcmp(lcdbuf, oldbuf) != 0) {//cap nhat lai man hinh khi lcdbuf thay doi
            lcd.backlight();
            lcdDisplay(lcdbuf, lcdRows);
            strcpy(oldbuf, lcdbuf);
          }
        }
        interval = 1;

 }else{//khi state=pause/stop
        lcd.clear();
        lcd.noBacklight();
        memset(oldbuf, 0, sizeof(oldbuf));
        interval = 1;
      }
    counter=0;  
      }       
    }
  }
