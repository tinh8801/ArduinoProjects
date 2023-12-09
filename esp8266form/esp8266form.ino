/*
 * ESP8266 (NodeMCU) Handling Web form data basic example
 * https://circuits4you.com
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h2>HTML Form ESP8266<h2>
<h3>Change Blink Speed by HTTP Server</h3>

<form action="/action_page">
  Led On
  <input type="text" name="ledon" value="">
  <br><br>
  Led Off
  <input type="text" name="ledoff" value="">
  <br><br>
  <input type="submit" value="Change">
</form>
<br><br><font color='red'>Reset Speed</font>
<form action="/test_page">
     <input type="submit" value="Reset">
</form> 

</body>
</html>
)=====";

//SSID and Password of your WiFi router
const char* ssid = "DDWRT";
const char* password = "12347890";
int ledon=500;
int ledoff=500;
ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
 String ledOn = server.arg("ledon"); 
 String ledOff = server.arg("ledoff"); 
 if (ledOn!="" && ledOff!=""){
 ledon = ledOn.toInt();
 ledoff = ledOff.toInt(); 
 }else{
  ledon=500;
  ledoff=500;
 }
 
 Serial.print("Led On: ");
 Serial.println(ledOn);

 Serial.print("Led Off: ");
 Serial.println(ledOff);
 
 String s = "<a href='/'><font size='15'>Go Back</font></a>";
 server.send(200, "text/html", s); //Send web page
}
void handleTest(){
  ledon=1000;
  ledoff=1000;
  Serial.print("Reset Speed");
  String s = "<a href='/'><font size='15'>Go Back</font></a>";
 server.send(200, "text/html", s); //Send web page
}
//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location
  server.on("/action_page", handleForm); //form action is handled here
  server.on("/test_page", handleTest);
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient(); //Handle client requests
  ledblink(ledon,ledoff);
}

void ledblink(int ledon,int ledoff){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(ledon);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(ledoff);    
}
