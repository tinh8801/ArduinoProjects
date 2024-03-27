/*Display Volumio Playing Status and Remote Control with ESP8266 and OLED 128x64*/
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/roboto.h>//Su dung font tu tao bang fontconvert
#include "volumio_logo.h"//Hinh anh splash screen

//========================================================
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//===============================================================

#define btnNext 4//D2 (GPIO4)
#define btnPlay 5//D1 (GPIO5)

enum BUTTONS {NONE, NEXT, PLAY};//Trang thai nut nhan
//#define lcdCols 16// So cot LCD
//#define lcdRows 2//So hang LCD
#define CYCLE_INTERVAL 1000
#define NUM_OF_PAGES 2
#define DISPLAY_EDGE 108
#define SCROLL_SPEED 6

const char* ssid     = "DDWRT";
const char* password = "12347890";
WiFiClient client;

long lastMillis = 0;
int interval = 0;
int retries = 0;
StaticJsonDocument<1024> doc;

enum ITEMS {PLAY_STATUS, TITLE, ARTIST, INFO, OTHER};
ITEMS item_to_get=OTHER;

BUTTONS checkButton(){//Kiem tra trang thai nut nhan
  BUTTONS kq=NONE;
  long current_time=millis();
  int count=0;
  if(digitalRead(btnNext)==LOW){//Xu ly debounce nut nhan
    while(millis()-current_time<100){
      if(digitalRead(btnNext)==LOW){
        count++;
      }
    }
    if(count>50){
      kq=NEXT;
    }else{
      kq=NONE;
    }
  }
  if(digitalRead(btnPlay)==LOW){//Xu ly debounce nut nhan
    while(millis()-current_time<100){
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


int bracketPos(const char* src){//tim vi tri dau mo ngoac (
  int pos=0;
  for(int i=0; i<strlen(src)-1; i++){
      if(src[i]=='('){
        pos=i;
        break;
          }
        }
  return pos;
}

char* seekConvert(long seekval){//doi seek (ms) sang phut, giay
  static char kq[5];
  int seconds=seekval/1000;
  int minutes=seconds/60;
  seconds=seconds%60;
  sprintf(kq, "%02d:%02d", minutes, seconds);// mm:ss
  return kq;
}

char* durationConvert(int durval){//doi duration (s) sang phut, giay
  static char kq[5];
  int minutes=durval/60;
  int seconds=durval%60;
  sprintf(kq, "%02d:%02d", minutes, seconds);// mm:ss
  return kq;
}

char* tracktypeUpper(const char* tracktype){//doi tracktype thanh chu IN HOA
  static char kq[5];
  for(int i=0; i<=strlen(tracktype); i++){
    kq[i] = toupper(tracktype[i]);
  }
  return kq;
}

int getItemJson(String line, ITEMS item, char* value){

/*
JSON tu Volumio:
{"status":"play","position":5,"title":"Matsuri","artist":"Kitaro","album":"",
"albumart":"/albumart?cacheid=853&path=%2FUSB%2FMUSIC%2FLossless&metadata=false",
"uri":"mnt/USB/MUSIC/Lossless/Matsuri.ape","trackType":"ape",
"seek":87239,"duration":543,"samplerate":"44.1 KHz","bitdepth":"16 bit","channels":2,"random":false,"repeat":true,
"repeatSingle":false,"consume":false,"volume":50,"disableVolumeControl":false,"mute":false,"stream":"ape",
"updatedb":false,"volatile":false,"service":"mpd"}
*/

    DeserializationError error = deserializeJson(doc, line);
    if (error) {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.f_str());
    //strcpy(value, "Invalid JSON");
    }else{
      memset(value, 0, sizeof(value));  
      const char* status=doc["status"];//"play"
      const char* title=doc["title"];//"My Love"
      const char* artist=doc["artist"];//"Westlife"
      const char* trackType = doc["trackType"]; // "flac"
      long seek = doc["seek"]; // 87239 (in ms)
      int duration = doc["duration"]; // 543 (in second)
      const char* samplerate = doc["samplerate"]; // "44.1 KHz"
       
      switch (item){
        case PLAY_STATUS:
          strcpy(value, status);
          break;

        case TITLE:
          if(strlen(title)>0){
            if(bracketPos(title)>0){//neu trong title co dau ngoac
              strncpy(value, title, bracketPos(title)-1); //copy chuoi truoc dau ngoac
            }else{
              strcpy(value, title);
                }     
            }
          break;

        case ARTIST:
          if(strlen(artist)>0){
            if(bracketPos(artist)>0){
              strncpy(value, artist, bracketPos(artist)-1);
            }else{
              strcpy(value, artist);    
              }     
          }else{
            strcpy(value, "No Artist"); 
          }
          break;

        case INFO:
          strcpy(value, seekConvert(seek));
          strcat(value, "/");
          strcat(value, durationConvert(duration));
          strcat(value, " ");
          strcat(value, tracktypeUpper(trackType));
          strcat(value, " ");
          strcat(value, samplerate);
          break;
        }
      }       
    return strlen(value);
 }
 

void fillBuffer(char* line, int len){//Neu line<16 ky tu thi bo sung bang ky tu 0x20 (Space)
  int sz = strlen(line);
  for (int i=sz;i<len;i++) {
    line[i] = 0x20;
    line[i+1]=0;
  }
}

void lcdDisplay(char* lcdbuf, int rows) {//xuat thong tin ra LCD
  char line[17];
  memset(line, 0, sizeof(line));//Set toan bo gia tri mang line thanh 0
  strncpy(line, lcdbuf, 16);
  fillBuffer(line, 16);
  //Serial.println("line1=[" + String(line) + "]");
  lcd.setTextSize(1);             // Normal 1:1 pixel scale
  lcd.setTextColor(SSD1306_WHITE);        // Draw white text
  lcd.cp437(true);
  lcd.setCursor(0, 0);
  lcd.println(line);//In 16 ky tu dau cua lcdbuf ra dong 1 LCD

  if (strlen(lcdbuf) > 16) {//Neu lcdbuf>16 ky tu thi in ra tiep phan con lai
    strncpy(line, &lcdbuf[16], 16);
  } else {
    strcpy(line," ");
  }
  fillBuffer(line, 16);
  //Serial.println("line2=[" + String(line) + "]");
  lcd.setCursor(0, 10);
  lcd.println(line);
  lcd.display();
  if (rows == 2) return;//Neu LCD 16x2 thi dung lai
  if (strlen(lcdbuf) > 32) {//Neu LCD 16x4, 20x4... thi tiep tuc in ra
    strncpy(line, &lcdbuf[32], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  //Serial.println("line3=[" + String(line) + "]");
  lcd.setCursor(0, 20);
  lcd.println(line);

  if (strlen(lcdbuf) > 48) {
    strncpy(line, &lcdbuf[48], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  //Serial.println("line4=[" + String(line) + "]");
  lcd.setCursor(0, 30);
  lcd.println(line);
}

void lcdRowDisplay(char* content, int rows=0) {//xuat thong tin ra LCD
  char line[40];
  memset(line, 0, sizeof(line));//Set toan bo gia tri mang line[40] ve 0
  strncpy(line, content, 39);
  //Serial.println("line1=[" + String(line) + "]");
  fillBuffer(line, 39);
  lcd.setTextSize(1);             // Normal 1:1 pixel scale
  lcd.setTextColor(SSD1306_WHITE);        // Draw white text
  lcd.setCursor(0, rows);
  lcd.println(line);
  lcd.display();
  
  //if (rows == 2) return;//Neu LCD 16x2 thi dung lai
}
  
void lcdRowDisplay2(char* content, int cols=0, int rows=0) {//xuat thong tin ra LCD
  char line[40];
  memset(line, 0, sizeof(line));//Set toan bo gia tri mang line[40] ve 0
  strcpy(line, content);
  fillBuffer(line, 39);
  lcd.setTextSize(1);             // Normal 1:1 pixel scale
  lcd.setTextColor(SSD1306_WHITE);        // Draw white text
  lcd.setCursor(cols, rows);
  lcd.println(line);
  lcd.display();
  //if (rows == 2) return;//Neu LCD 16x2 thi dung lai
}


//===================================================

void setup() {
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
   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!lcd.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0, volumio_logo, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);//hien thi logo Volumio
  lcd.display();
  delay(2000);
  lcd.clearDisplay();
  lcd.setFont(&Roboto_Regular6pt7b);
  lcd.setTextSize(1);             // Normal 1:1 pixel scale
  lcd.setTextColor(SSD1306_WHITE);        // Draw white text
  lcd.setCursor(0, 8);             // Start at top-left corner
  lcd.println(F("WiFi Connected"));
  lcd.setCursor(0, 24);
  lcd.println(F("IP Address: "));
  lcd.setCursor(0, 40);
  lcd.println(WiFi.localIP());
  lcd.display();
  delay(1000);
  pinMode(btnNext,INPUT_PULLUP);
  pinMode(btnPlay,INPUT_PULLUP);  
}

//=======================================================

int lcd_page;
int cnt;
void loop() {
  static int counter = 0;
  char state[10];//Trang thai Volumio (play, pause, stop)
  char lcdbuf[80] = {0};//noi dung se xuat ra LCD
  static char oldbuf[80] = {0};
  String payload="";
  
if(checkButton()==NEXT){
      HTTPClient http;
      http.begin(client,F("http://192.168.2.8/api/v1/commands?cmd=next"));
      int httpCode=http.GET();
      if(httpCode>0){
        //Serial.println("Button NEXT Pressed");
        }          
  }
  
 if(checkButton()==PLAY){
      HTTPClient http;
      http.begin(client,F("http://192.168.2.8/api/v1/commands?cmd=toggle"));
      int httpCode=http.GET();
      if(httpCode>0){
        //Serial.println("Button PLAY/PAUSE Pressed");
        }
  }
       
  long now = millis();
  if (now < lastMillis) lastMillis = now; // millis is overflow
  if (now - lastMillis > CYCLE_INTERVAL) {//vong lap 1000ms
    lastMillis = now;
    counter++;
   
    HTTPClient http;
    http.begin(client,F("http://192.168.2.8/api/v1/getState"));
    int httpCode=http.GET();
    if(httpCode > 0){//neu ket noi duoc voi Volumio
        retries=0;
	      payload=http.getString();//Nhan ve chuoi JSON
        item_to_get=PLAY_STATUS;
        //Serial.println("JSON: "+payload);
        //int stateLen=getItem(payload, "status", state, sizeof(state));
        int stateLen=getItemJson(payload, item_to_get, state);
        //Serial.println("state=" + String(state));
        } else{//khong ket noi duoc thi xoa LCD va reset ESP8266
          lcd.clearDisplay();
          lcd.display();
          delay(10*1000);
	        retries+=1;
	        if(retries>20){
              ESP.restart();
		            } 
            }    
       
    if (counter > interval) {//bat dau chu ky in noi dung ra LCD
        if(strcmp(state,"play") == 0) {//Neu Volumio dang play thi xu ly thong tin va in ra LCD
          char artist[40];
          char info[40];
          char title[40];
          int artistLen;
          int titleLen;
          int infoLen;
          
          item_to_get=TITLE;
        //titleLen = getItem(payload, "title", title, sizeof(title));
          memset(title, 0, sizeof(title));
          titleLen = getItemJson(payload, item_to_get, title);

          item_to_get=ARTIST;
        //artistLen = getItem(payload, "artist", artist, sizeof(artist));
          memset(artist, 0, sizeof(artist));
          artistLen = getItemJson(payload, item_to_get, artist);
        //Serial.println("Artist=" + String(artist));
          //if((sizeof(artist)/sizeof(artist[0]))<1){
              //strcpy(artist,"*");
           // }
          item_to_get=INFO;
          memset(info, 0, sizeof(info));
          infoLen = getItemJson(payload, item_to_get, info);
        //Serial.println("Title=" + String(title));

        memset(lcdbuf, 0, sizeof(lcdbuf));
        if (titleLen > 0) {
          strcpy(lcdbuf, title);
          if(artistLen > 0){
          strcat(lcdbuf, "::");
          strcat(lcdbuf, artist);//tao mang lcdbuf chua Title va Artist
          }      
        }
     
        char lcdbuf_title[40];
        char lcdbuf_artist[40];
        char splitter[4];
        strcpy(splitter, ">>>");
        //char lcdbuf_part3[20];
        memset(lcdbuf_title, 0, sizeof(lcdbuf_title));
        if (titleLen > 0) {
          strcpy(lcdbuf_title, title);
        }
        if(artistLen > 0){
          strcpy(lcdbuf_artist, artist);
          }
        
        //strncpy(lcdbuf_part1, lcdbuf, 19);
        //strncpy(lcdbuf_part2, &lcdbuf[19], 19);
        //strncpy(lcdbuf_part3, &lcdbuf[38], 19);
    
        lcd_page+=1;
        cnt+=SCROLL_SPEED;
        if (cnt>DISPLAY_EDGE){
          cnt=0;
        }
        if (lcd_page>NUM_OF_PAGES){
          lcd_page=0;
        }
        //Serial.println("split_parts=[" + String(split_parts) + "]");
        if (strlen(lcdbuf) > 0) {
          if (strcmp(lcdbuf, oldbuf) != 0) {//neu lcdbuf co thay doi thi cap nhat LCD
            lcd.clearDisplay();
            //lcdDisplay(lcdbuf, lcdRows);
            lcdRowDisplay(lcdbuf, 8);
            lcdRowDisplay2(splitter, 0, 36);
            lcdRowDisplay(info, 48);
            strcpy(oldbuf, lcdbuf);
            cnt=0;
          }else{
            //lcdRow1Display(lcdbuf, lcdRows);
            lcdRowDisplay(info, 48);
            lcdRowDisplay2(splitter, cnt, 36);
            if(lcd_page==1) {
             lcdRowDisplay(lcdbuf_title, 8);
            }else if (lcd_page==2){ 
              lcdRowDisplay(lcdbuf_artist, 8);
            }
            //}else if (lcd_page==3){
             // lcdRow1Display(lcdbuf_part3, lcdRows);    
            //}
            
             
          }
        }
        interval = 1;

 }else{//state=pause/stop
    lcd.clearDisplay();
   lcd.display();
    memset(oldbuf, 0, sizeof(oldbuf));
    interval = 1;
      }
  counter=0;//ket thuc chu ky hien thi
  lcd.clearDisplay();    
      }
    }//ket thuc vong lap 1000ms
interval=0;

  }

