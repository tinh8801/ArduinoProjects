/*
  Vision Old: CongToDien_1Pha_PZEM_LCD_OLED
  Vision New: CongToDien_1Pha_PZEM_LCD_OLED_V2

 *** CÔNG TƠ ĐIỆN TỬ WIFI 1 PHA + LCD1602 I2C or OLED 0.96 inch ***
  Youtube  : https://www.youtube.com/c/403ERRORVN
  Fanpage  : https://www.facebook.com/403ERRORVN
  Blog     : https://thichchetaotv.blogspot.com/

  Date     : 26/07/2022
  Update   : 03/03/2023

  History:
  - 22/09/2022: + Thêm Time & Date trên Màn hình hiển thị
                + Thêm thông số tiêu thụ (hôm qua, hôm nay, tháng trước, tháng này)


  Library:
  Blynk           : https://github.com/blynkkk/blynk-library
  PZEM004T        : https://github.com/mandulaj/PZEM-004T-v30
  WiFiManager     : https://github.com/tzapu/WiFiManager
  Adafruit_SSD1306: https://github.com/adafruit/Adafruit_SSD1306
  Adafruit_GFX.h  : https://github.com/adafruit/Adafruit-GFX-Library

  ***************** LOG ********************
  V10: Reset Energy       (Buton)

  ------------------------------------------
  V0 : Voltage            (V)
  V1 : Current            (A)
  V2 : Power              (W)
  V3 : Energy             (kWh)
  V4 : Freq               (Hz)
  V5 : pF 
  V6 : Energy_today       (kWh)
  V7 : Energy_Yesterday   (kWh)
  V8 : Energy_This_month  (kWh)
  V9 : Energy_Last_month  (kWh)
  V11: WiFi strength      (%)
  V12: Time               (String)
  V13: Date               (String)

  Event :
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1         // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//========== SETTING WiFi ==========//
#define BLYNK_TEMPLATE_ID "TMPL6SZnkyACR"
#define BLYNK_TEMPLATE_NAME "cong to dien thong minh"
#define BLYNK_AUTH_TOKEN "U6zQmwKzfE9GSfHIV_Jrc1Vx8dVDUstw"

#define Device_Name "CONG TO DIEN 1 PHA"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>

char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;
WidgetRTC rtc;

#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

#define PZEM_RX_PIN 2   // GPIO2 - D4 ESP8266
#define PZEM_TX_PIN 0   // GPIO0 - D3 ESP8266
#define configWifi D0   //         D0 - Config WiFi 

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);



WiFiManager wifiManager;

float
voltage  ,
current  ,
power    ,
energy   ,
frequency,
pf       ,
Energy_today,
Energy_Yesterday,
Energy_This_month,
Energy_Last_month,
energy_log_day,
energy_log_month;

int
conv1  = 0,           // SYSTEM PARAMETER
conv2  = 0,
d_hour = 0,
d_day  = 0;

String
currentTime,
currentDate;

const unsigned char Logo [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x03, 0xc0, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x03, 0xc0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xde, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x3f, 0x1f, 0xf9, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x7f, 0x1f, 0xf9, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0xff, 0x3f, 0xf1, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x81, 0xff, 0x3f, 0xf1, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x83, 0xfe, 0x3e, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x87, 0xfe, 0x3f, 0xe1, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x8f, 0xbe, 0x3f, 0xe1, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x9f, 0xff, 0x7f, 0xc1, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x9f, 0xff, 0x7f, 0xc1, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x9f, 0xff, 0x7c, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x9f, 0xff, 0x7f, 0xe1, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x7c, 0x7f, 0xc1, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x7c, 0xff, 0xc1, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0xf8, 0xff, 0xc1, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x03, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  currentTime = String(hour()) + ":" + minute() + ":" + second();
  currentDate = String(day()) + "/" + month() + "/" + year();
  // Serial.print("Current time: ");
  // Serial.print(currentTime);
  // Serial.print(" ");
  // Serial.print(currentDate);
  // Serial.println();
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}

BLYNK_WRITE(V10) {         // Button Widget V6
  if (param.asInt() == 1) {
    pzem.resetEnergy();
    Energy_Yesterday = 0;
    Energy_Last_month = 0;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(configWifi, INPUT);
  EEPROM.begin(512);
  loadSettings();

  //lcd.init();  //lcd.setBacklight(HIGH);

  display.begin(SCREEN_ADDRESS, true);
    
  wifiManager.setConfigPortalTimeout(60); // 1 minute

  //  wifiManager.autoConnect("CongToDien1Pha");

  if (!wifiManager.autoConnect(Device_Name)) {
    Serial.println("failed to connect and hit timeout");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Dang Ket Noi....");
    display.display();
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Wifi connecting to ");
  display.println( WiFi.SSID().c_str() );

  Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());

  display.println("\nConnecting");
  display.display();

  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    display.print(".");
    display.display();
  }

  display.clearDisplay();
  display.display();
  display.setCursor(0, 0);

  display.println("Wifi Connected!");
  display.print("IP:");
  display.println(WiFi.localIP() );
  display.display();
  delay(1000);

  display.clearDisplay();
  display.display();
 

  display.drawBitmap(0, 0, Logo, 128, 64, SH110X_WHITE);
  display.display();
  
  delay(3000);
  display.clearDisplay();
  
  display.display();

  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 1 seconds
  timer.setInterval(1000L, clockDisplay);

} // end setup

void loop() {
  if (digitalRead(configWifi) == HIGH) {
    delay(30);
    if (digitalRead(configWifi) == HIGH) {
      delay(3000);
      if (digitalRead(configWifi) == HIGH) {
        wifiManager.resetSettings();
        delay(1000);
        ESP.restart();
      }
    }
  }

  voltage = pzem.voltage();
  current = pzem.current();
  power = pzem.power();
  energy = pzem.energy();
  frequency = pzem.frequency();
  pf = pzem.pf();

  //SerialMonitor();
  OLED13();           // Select if using OLED 0.96 display
  // LCD1602();           // Select if using LCD 1602 display
  energy_calculate();
  sendBlynk();
  Blynk.run();
  timer.run();

} //end loop

void sendBlynk()
{
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power  );
  Blynk.virtualWrite(V3, energy );
  Blynk.virtualWrite(V4, frequency);
  Blynk.virtualWrite(V5, pf);
  Blynk.virtualWrite(V6, Energy_today);
  Blynk.virtualWrite(V7, Energy_Yesterday);
  Blynk.virtualWrite(V8, Energy_This_month);
  Blynk.virtualWrite(V9, Energy_Last_month);
  // V10 - Reset Energy
  Blynk.virtualWrite(V11, 2 * (WiFi.RSSI() + 100));
  Blynk.virtualWrite(V12, currentTime);
  Blynk.virtualWrite(V13, currentDate);

}

void OLED13()
{
  if (isnan(voltage)) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);              // Start at top-left corner
    display.print("          ");
    display.setCursor(10, 10); display.print("   NOT   ");
    display.setCursor(10, 30); display.print("CONNECTED");
    display.display();
  }
  else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);  // Draw white text
    display.setCursor(0, 0);              // Start at top-left corner
    display.print("  DO SO DIEN MINH ");
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print(power);            display.print("W");
    display.setTextSize(1);                  // Normal 1:1 pixel scale
    display.setCursor(0, 35);
    display.print(voltage);          display.print("V | ");
    display.print(current);          display.print(".AMPE");
    display.setCursor(0, 45);
    display.print(frequency, 1);     display.print("Hz/");
    display.print(pf);               display.print("-");
    display.print(energy);           display.print("kWh");

    display.setCursor(0, 55);
    display.print(currentTime);   display.print(" | ");
    display.print(currentDate);
    display.display();
  }
} // end hienthiLCD


void SerialMonitor()
{
  // Check if the data is valid
  if (isnan(voltage)) {
    Serial.println("Error reading voltage");
  } else if (isnan(current)) {
    Serial.println("Error reading current");
  } else if (isnan(power)) {
    Serial.println("Error reading power");
  } else if (isnan(energy)) {
    Serial.println("Error reading energy");
  } else if (isnan(frequency)) {
    Serial.println("Error reading frequency");
  } else if (isnan(pf)) {
    Serial.println("Error reading power factor");
  } else {

    // Print the values to the Serial console
    Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
    Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
    Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
    Serial.print("Energy: ");       Serial.print(energy, 3);     Serial.println("kWh");
    Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
    Serial.print("PF: ");           Serial.println(pf);
  }
  Serial.println();
}  // end SerialMonitor

void energy_calculate() {
  // int T_hour = hour();
  // int D_day  = day();
  Energy_today = energy - energy_log_day;
  if (hour() != 0) {
    d_hour = 1;
  }

  if ((hour() == 0)  && (d_hour == 1) ) {  // sang ngày mới
    Energy_Yesterday = Energy_today;
    Energy_today     = 0;
    energy_log_day   = energy;
    d_hour           = 0;
  }

  Energy_This_month = energy - energy_log_month;
  if (day() != 1) {
    d_day = 1;
  }

  if ((day() == 1) && (d_day == 1)) {
    Energy_Last_month = Energy_This_month;
    Energy_This_month = 0;
    energy_log_month  = energy;
    d_day = 0;
  }

  Serial.print("Hour: "); Serial.print(hour());  Serial.print(" | d_hour: "); Serial.println(d_hour);
  saveSettings();
}



void loadSettings() {
  Energy_today       = EEPROM.read(0) + (EEPROM.read(1) * .01);
  Energy_Yesterday   = EEPROM.read(2) + (EEPROM.read(3) * .01);
  Energy_This_month  = EEPROM.read(4) + (EEPROM.read(5) * .01);
  Energy_Last_month  = EEPROM.read(6) + (EEPROM.read(7) * .01);
}

void saveSettings() {
  conv1 = Energy_today * 100;
  conv2 = conv1 % 100;
  EEPROM.write(0, Energy_today);
  EEPROM.write(1, conv2);

  conv1 = Energy_Yesterday * 100;
  conv2 = conv1 % 100;
  EEPROM.write(2, Energy_Yesterday);
  EEPROM.write(3, conv2);

  conv1 = Energy_This_month * 100;
  conv2 = conv1 % 100;
  EEPROM.write(4, Energy_This_month);
  EEPROM.write(5, conv2);

  conv1 = Energy_Last_month * 100;
  conv2 = conv1 % 100;
  EEPROM.write(6, Energy_Last_month);
  EEPROM.write(7, conv2);
  EEPROM.commit();                     //Saves setting changes to flash memory
}
void saveAutoloadSettings() {
  //  EEPROM.write(11,flashMemLoad);       //STORE: Enable autoload
  //EEPROM.commit();                     //Saves setting changes to flash memory
}
