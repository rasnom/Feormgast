#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

#define LED 2

const char *SSID = "Feormgast";
const char *PASSWORD = AP_WIFI_PASSWORD;

WiFiServer server(80);

String header;

bool lightOn = false;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long lightOnTime = 0;
const long wifiTimeoutTime = 2000; // milliseconds
const long lightDuration = 3000;


void setup() {
  Serial.begin(115200);

  // Create Wifi Access Point
  Serial.print("Creating network ");
  Serial.println(SSID);
  Serial.println(PASSWORD);
  WiFi.softAP(SSID, PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(" at IP: ");
  Serial.println(IP);

  server.begin();

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

void loop() {
  WiFiClient client = server.available();

  if (lightOn && millis() - lightOnTime >= lightDuration) {
    Serial.println("Light turns itself off");
    digitalWrite(LED, LOW);
    lightOn = false;
  }

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= wifiTimeoutTime ) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          // After the end of a line, currentLine gets wiped
          // If it is alread wiped then that double newline
          // marks the end of the message.
          if (currentLine.length() == 0) {
            // Response code
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Turn LED on or off
            if (header.indexOf("GET /light/on") >= 0) {
              Serial.println("Turn light ON");
              digitalWrite(LED, HIGH);
              lightOn = true;
              lightOnTime = currentTime;
            } 
            else if (header.indexOf("GET /light/off") >= 0) {
              Serial.println("Turn light OFF");
              digitalWrite(LED, LOW);
              lightOn = false;
            }

            // HTML header start
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off button 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1><Feormgast Web Server</h1>");

            // Current state
            client.print("<p>LED Light State - ");
            if (lightOn) {
              client.println("ON</p>");
              client.println("<p><a href=\"/light/off\"><button class=\"button button2\">Turn OFF</button></a></p>");
            } else {
              client.println("OFF</p>");
              client.println("<p><a href=\"/light/on\"><button class=\"button\">Turn ON</button></a></p>");
            }
            client.println("</body></html>");

            client.println(); // consecutive newline indicates end of message
            Serial.println("*** break");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // Clear header & close connection
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");    
  }
 }