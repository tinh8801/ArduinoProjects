/*
 *  MPD Client Example for ESP8266/ESP32
 *
 *  LED Matrix Pin -> ESP8266 Pin
 *  Vcc            -> 3v
 *  Gnd            -> Gnd
 *  DIN            -> D7
 *  CS             -> D8
 *  CLK            -> D5
 */
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else        
#include <WiFi.h>
#endif
#include <SPI.h>
#include <MD_MAX72xx.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 14
#define DATA_PIN 13
#define CS_PIN 15
#define CHAR_SPACING  1 // pixels between characters

int wait = 100; // In milliseconds
int width = 5 + CHAR_SPACING; // The font width is 5 pixels

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char *ssid     = "DDWRT";
const char *password = "12347890";
uint16_t port = 6600;
IPAddress host(192,168,2,8); // ip or dns


// Use WiFiClient class to create TCP connections
WiFiClient client;

long lastMillis = 0;
int interval = 0;

int mpc_connect(IPAddress host, int port) {
  char smsg[40];
  char rmsg[40];

  if (!client.connect(host, port)) {
      Serial.println(F("Connection failed"));
      return 0;
  }

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('\0');
  //Serial.print("[");
  //Serial.print(line);
  //Serial.println("]");
  //Serial.println("length()=" + String(line.length()));
  line.toCharArray(rmsg, line.length()+1);
  //Serial.println("strlen()=" + String(strlen(rmsg)));
  rmsg[line.length()-1] = 0;
  Serial.println("rmsg=[" + String(rmsg) + "]");
  if (strncmp(rmsg,"OK",2) == 0) return 1;
  return 0;
}

int mpc_command(char * buf) {
  char smsg[40];
  char rmsg[40];
  sprintf(smsg,"%s\n",buf);
  client.print(smsg);
  Serial.println("smsg=[" + String(buf) + "]");

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('\0');
  //Serial.print("[");
  //Serial.print(line);
  //Serial.println("]");
  //Serial.println("length()=" + String(line.length()));
  line.toCharArray(rmsg, line.length()+1);
  //Serial.println("strlen()=" + String(strlen(rmsg)));
  rmsg[line.length()-1] = 0;
  Serial.println("rmsg=[" + String(rmsg) + "]");
  if (strcmp(rmsg,"OK") == 0) return 1;
  return 0;
}

void mpc_error(char * buf) {
  Serial.print(F("mpc command error:"));
  Serial.println(buf);
  while(1) {}
}


int getItem(String line,String item, char * value, int len) {
  int pos1,pos2,pos3;
  Serial.println("item=[" + String(item) + "]");
  pos1=line.indexOf(item);
  //Serial.println("pos1=" + String(pos1));
  String line2;
  line2 = line.substring(pos1);
  pos2=line2.indexOf(":");
  pos3=line2.indexOf(0x0a);
  //Serial.println("pos2=" + String(pos2));
  //Serial.println("pos3=" + String(pos3));
  String line3;
  line3 = line2.substring(pos2+1,pos3);
  //Serial.println("line3=[" + line3 + "]");
  string2char(line3, value, len);
  Serial.println("value=[" + String(value) + "]");
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

void setup() {
  mx.begin();
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Wait for WiFi..."));
  WiFi.begin(ssid, password);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    cnt++;
    if ((cnt % 60) == 0) Serial.println();
  }

  Serial.println(F("OK"));
  Serial.println(F("WiFi connected"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  while(1) {
    Serial.print(F("Connecting to "));
    Serial.println(host);
  //if (mpc_connect(host, port) == 0) mpc_error("connect");
    if (mpc_connect(host, port) == 1) break;
    delay(10*1000);
  }

  // set up the 8×32 LED Matrix MAX7219
  
}

void loop() {
  static int counter = 0;
  String line;
  char state[40];
  char smsg[40];
  char lcdbuf[80] = {0};
  static char oldbuf[80] = {0};
  static int offset = 0;
  
  if (!client.connected()) {
    Serial.println(F("Server disconnected"));
    delay(10*1000);
    ESP.restart();
  }

  long now = millis();
  if (now < lastMillis) lastMillis = now; // millis is overflow
  if (now - lastMillis > 1000) {
    lastMillis = now;
    counter++;
    if (counter > interval) {
      sprintf(smsg,"status\n");
      client.print(smsg);
      Serial.println(F("status"));
    
      //read back one line from server
      client.setTimeout(1000);
      line = client.readStringUntil('\0');
      //Serial.println("status=[" + line + "]");
      Serial.println(F("state=" + String(getItem(line, "state:", state, sizeof(state)))));

      if (strcmp(state,"play") == 0) {
        sprintf(smsg,"currentsong\n");
        client.print(smsg);
        //read back one line from server
        client.setTimeout(1000);
        line = client.readStringUntil('\0');
        Serial.println("currentsong=[" + line + "]");

        char artist[40];
        char title[40];
        int artistLen;
        int titleLen;
        artistLen = getItem(line, "Artist:", artist, sizeof(artist));
        Serial.println("Artist=" + String(artistLen));
        titleLen = getItem(line, "Title:", title, sizeof(title));
        Serial.println("Title=" + String(titleLen));
        memset(lcdbuf, 0, sizeof(lcdbuf));
        if (artistLen > 0 && titleLen > 0) {
          strcpy(lcdbuf,title);
          strcat(lcdbuf, " - ");
          strcat(lcdbuf,artist);
        } else if (artistLen > 0 && titleLen == 0) {
          strcpy(lcdbuf, artist);
        } else if (artistLen == 0 && titleLen > 0) {
          strcpy(lcdbuf, title);
        }

        Serial.println("lcdbuf=[" + String(lcdbuf) + "]");
        if (strlen(lcdbuf) > 0) {
          scrollText(lcdbuf,100);
          strcpy(oldbuf, lcdbuf);
        }
        interval = 0;

      }  else {  // state = stop
        mx.clear();
        memset(oldbuf, 0, sizeof(oldbuf));
        interval = 1;
      }
      counter=0;
    }
  }
}
