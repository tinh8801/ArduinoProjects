#include <ESP8266WiFi.h>
const char* ssid = "DDWRT";
const char* password = "12347890";

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Dang ket noi den mang… ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Da ket noi WiFi");
  server.begin();
  Serial.println("Web server dang khoi dong. Vui long doi dia chi IP…");
  delay(1000);
  Serial.println(WiFi.localIP());
}

void loop()
{
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("Co client moi");
    boolean blank_line = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (c == '\n' && blank_line)
        {      
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html><head></head><body>");
          client.println("<h1>ESP8266 Web Server</h1>");
          client.println("<h2>Content</h2><h3>Nhiet do theo do C: ");
          client.println(37);
          client.println("*C</h3><h3>Nhiet do theo do F: ");
          client.println(100);
          client.println("*F</h3><h3>Do am: ");
          client.println(60);
          client.println("%</h3><h3>");
          client.println("<h2>huy thang</h2>");
          client.println("</body></html>");

          break;
        }
        if (c == '\n')
        {
          blank_line = true;
        }
        else if (c != '\r')
        {
          blank_line = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("Ngat ket noi client.");
  }
}