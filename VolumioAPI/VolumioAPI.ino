/*Display Volumio Playing Status and Remote Control with ESP8266 and LCD 16x02*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_74HC595.h>
#include <ArduinoJson.h>
//Ket noi LCD:    74HC595             NodeMCU
#define DS 14 //   14                 D5 (GPIO14)
#define SHCP 12//  11                 D6 (GPIO12)
#define STCP 13//  12                 D7 (GPIO13)
#define RS 7
#define E 6
#define D4 4
#define D5 3
#define D6 2
#define D7 1
#define BACKLIGHT 15//D8 (GPIO15) LCD Backlight
LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7, BACKLIGHT);//Khai bao LCD

#define btnNext 4//D2 (GPIO4)
#define btnPlay 5//D1 (GPIO5)

enum BUTTONS {NONE, NEXT, PLAY};//Trang thai nut nhan
#define lcdCols 16// So cot LCD
#define lcdRows 2//So hang LCD
#define CYCLE_INTERVAL 1000
#define NUM_OF_PAGES 3

const char* ssid     = "DDWRT";
const char* password = "12347890";
WiFiClient client;

long lastMillis = 0;
int interval = 0;
int retries=0;
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

/*
JSON tu Volumio:
{"status":"play","position":5,"title":"Matsuri","artist":"Kitaro","album":"",
"albumart":"/albumart?cacheid=853&path=%2FUSB%2FMUSIC%2FLossless&metadata=false",
"uri":"mnt/USB/MUSIC/Lossless/Matsuri.ape","trackType":"ape",
"seek":87239,"duration":543,"samplerate":"44.1 KHz","bitdepth":"16 bit","channels":2,"random":false,"repeat":true,
"repeatSingle":false,"consume":false,"volume":50,"disableVolumeControl":false,"mute":false,"stream":"ape",
"updatedb":false,"volatile":false,"service":"mpd"}
*/
char* seekConvert(long seekval){//chuyen seek (ms) sang phut giay
  static char kq[5];
  int seconds=seekval/1000;
  int minutes=seconds/60;
  seconds=seconds%60;
  sprintf(kq, "%02d:%02d", minutes, seconds);// mm:ss
  return kq;
}

int getItemJson(String line, ITEMS item, char* value){
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
      //int duration = doc["duration"]; // 543 (in second)
      const char* samplerate = doc["samplerate"]; // "44.1 KHz"
       
      switch (item){
        case PLAY_STATUS:
          strcpy(value, status);
          break;

        case TITLE:
          if(strlen(title)>0){
            if(bracketPos(title)>0){//neu trong chuoi co dau ngoac
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
          strcat(value, " ");
          strcat(value, trackType);
          strcat(value, " ");
          strcat(value, samplerate);
          break;
        }
      }       
    return strlen(value);
 }
 
/*int getItem(String line, String item, char * value, int len) {//phan tich chuoi tra ve tu getState
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
*/

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
  lcd.setCursor(0, 0);
  lcd.print(line);//In 16 ky tu dau cua lcdbuf ra dong 1 LCD

  if (strlen(lcdbuf) > 16) {//Neu lcdbuf>16 ky tu thi in ra tiep phan con lai
    strncpy(line, &lcdbuf[16], 16);
  } else {
    strcpy(line," ");
  }
  fillBuffer(line, 16);
  //Serial.println("line2=[" + String(line) + "]");
  lcd.setCursor(0, 1);
  lcd.print(line);

  if (rows == 2) return;//Neu LCD 16x2 thi dung lai
  if (strlen(lcdbuf) > 32) {//Neu LCD 16x4, 20x4... thi tiep tuc in ra
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

void lcdRow1Display(char* line1, int rows) {//xuat thong tin ra row1 LCD
  char line[17];
  memset(line, 0, sizeof(line));//Set toan bo gia tri mang line[17] ve 0
  strncpy(line, line1, 16);//copy 16 ky tu cua lcdbuf vao mang line
  //Serial.println("line1=[" + String(line) + "]");
  fillBuffer(line, 16);
  lcd.setCursor(0, 0);
  lcd.print(line);//In ra row 1 LCD
  if (rows == 2) return;//Neu LCD 16x2 thi dung lai
}
  //if (strlen(lcdbuf) > 16) {//Neu lcdbuf>16 ky tu thi in ra tiep
   // strncpy(line, &lcdbuf[16], 16);//copy 16 ky tu tiep theo cua lcdbuf
  //} else {
    //strcpy(line," ");
  //}
  //fillBuffer(line, 16);
  //Serial.println("line2=[" + String(line) + "]");
void lcdRow2Display(char* line2, int rows) {//xuat thong tin ra row2 LCD
  char line[17];
  memset(line, 0, sizeof(line));//Set toan bo gia tri mang line[17] ve 0
  strcpy(line, line2);
  fillBuffer(line, 16);
  lcd.setCursor(0, 1);
  lcd.print(line);//In ra row 2 LCD

  if (rows == 2) return;//Neu LCD 16x2 thi dung lai
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
  lcd.begin(lcdCols, lcdRows);//khoi tao LCD
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(F("Volumio Display"));
  lcd.setCursor(0,1);
  lcd.print(F("WiFi Connected"));
  delay(2000);
  lcd.noBacklight();
  lcd.clear();
  pinMode(btnNext,INPUT_PULLUP);
  pinMode(btnPlay,INPUT_PULLUP);  
}

//=======================================================

int lcd_page;//
void loop() {
  // put your main code here, to run repeatedly:
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
    if(httpCode>0){//neu ket noi duoc voi Volumio
        retries=0;
	      payload=http.getString();//Nhan ve chuoi JSON
        item_to_get=PLAY_STATUS;
        //Serial.println("JSON: "+payload);
        //int stateLen=getItem(payload, "status", state, sizeof(state));
        int stateLen=getItemJson(payload, item_to_get, state);
        //Serial.println("state=" + String(state));
        } else{//khong ket noi duoc thi xoa LCD va reset ESP8266
          lcd.clear();
          lcd.noBacklight();
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
          if((sizeof(artist)/sizeof(artist[0]))<1){
              strcpy(artist,"*");
            }
          item_to_get=INFO;
          memset(info, 0, sizeof(info));
          infoLen = getItemJson(payload, item_to_get, info);
        //Serial.println("Title=" + String(title));
       /* int t=0;
        int a=0;
        for(int i=0;i<sizeof(title)-1;i++){
          if(title[i]=='('){
            t=i-1;
            break;
          }
        for(int i=0;i<sizeof(artist)-1;i++){
        }
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
        if (titleLen > 0) {
          strcpy(lcdbuf, title);
          if(artistLen > 0){
          strcat(lcdbuf, "::");
          strcat(lcdbuf, artist);//tao mang lcdbuf chua Title va Artist
          }      
        }
        
        char lcdbuf_part1[17];
        char lcdbuf_part2[17];
        char lcdbuf_part3[17];
        
        strncpy(lcdbuf_part1, lcdbuf, 16);
        strncpy(lcdbuf_part2, &lcdbuf[16], 16);
        strncpy(lcdbuf_part3, &lcdbuf[32], 16);
    
        lcd_page+=1;
        if (lcd_page>NUM_OF_PAGES){
          lcd_page=0;
        }
        //Serial.println("split_parts=[" + String(split_parts) + "]");
        if (strlen(lcdbuf) > 0) {
          if (strcmp(lcdbuf, oldbuf) != 0) {//neu lcdbuf co thay doi thi cap nhat LCD
            lcd.clear();
            lcd.backlight();
            //lcdDisplay(lcdbuf, lcdRows);
            lcdRow1Display(lcdbuf, lcdRows);
            lcdRow2Display(info, lcdRows);
            strcpy(oldbuf, lcdbuf);
          }else{
            lcd.clear();
            lcdRow2Display(info, lcdRows);
            if(lcd_page==1) {
              lcdRow1Display(lcdbuf_part1, lcdRows);
            }else if (lcd_page==2){ 
              lcdRow1Display(lcdbuf_part2, lcdRows);   
            }else if (lcd_page==3){
              lcdRow1Display(lcdbuf_part3, lcdRows);    
            }    
          }
        }
        interval = 1;

 }else{//state=pause/stop
    lcd.clear();
    lcd.noBacklight();
    memset(oldbuf, 0, sizeof(oldbuf));
    interval = 1;
      }
  counter=0;//ket thuc chu ky hien thi     
      }
    }//ket thuc vong lap 1000ms
interval=0;
  }
