#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

#define LED 2
#define OPENPIN 22
#define CLOSEPIN 23

const char *SSID = "Feormgast";
const char *PASSWORD = AP_WIFI_PASSWORD;

WiFiServer server(80);

String header;

bool motorOn = false;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long motorOnTime = 0;
const long wifiTimeoutTime = 2000; // milliseconds
const long motorDuration = 20000;


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
  pinMode(OPENPIN, OUTPUT);
  digitalWrite(OPENPIN, LOW);
  pinMode(CLOSEPIN, OUTPUT);
  digitalWrite(CLOSEPIN, LOW);}

void loop() {
  WiFiClient client = server.available();

  if (motorOn && millis() - motorOnTime >= motorDuration) {
    digitalWrite(OPENPIN, LOW);
    digitalWrite(CLOSEPIN, LOW);
    digitalWrite(LED, LOW);
    motorOn = false;
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
            } 
            else if (header.indexOf("GET /light/off") >= 0) {
              Serial.println("Also turn light OFF");
              digitalWrite(LED, LOW);
            }  
            else if (header.indexOf("GET /door/open") >= 0) {
              Serial.println("Opening door...");
              digitalWrite(LED, HIGH);
              digitalWrite(OPENPIN, HIGH);
              motorOn = true;
              motorOnTime = currentTime;
            } 
            else if (header.indexOf("GET /door/close") >= 0) {
              Serial.println("Closing Door");
              digitalWrite(LED, HIGH);
              digitalWrite(CLOSEPIN, HIGH);
              motorOn = true;
              motorOnTime = currentTime;
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

            // light controls 
            client.print("<p>LED Light State - ");
            if (motorOn) {
              client.println("ON</p>");
            } else {
              client.println("OFF</p>");
            }
            client.println("<p><a href=\"/light/on\"><button class=\"button\">Turn ON</button></a></p>");
            client.println("<p><a href=\"/light/on\"><button class=\"button button2\">ALSO ON</button></a></p>");

            // motor controls
            client.println("<h2>Chicken Coop Door</h2>");
            client.println("<p><a href=\"/door/open\"><button class=\"button\">OPEN</button></a></p>");
            client.println("<p><a href=\"/door/close\"><button class=\"button\">CLOSE</button></a></p>");

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