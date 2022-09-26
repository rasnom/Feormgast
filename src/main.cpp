#include <Arduino.h>
#include <WiFi.h>
#include <Webserver.h>
// #include <HTTPUpdateServer.h>
#include "secrets.h"

#define LED 2
#define OPENPIN 22
#define CLOSEPIN 23

const char *SSID = "Feormgast";
const char *PASSWORD = AP_WIFI_PASSWORD;

WebServer server(80);
// HTTPUpdateServer httpUpdater;

String header;

bool motorOn = false;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long motorOnTime = 0;
const long wifiTimeoutTime = 2000; // milliseconds
const long motorDuration = 5000;

const char* serverIndex = 
  "<html>"
    "<head>"
      "<style>"
        "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
        ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;"
        "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
        ".button2 {background-color: #555555;}"
      "</style>"
    "</head>"

    "<body>"
      "<h1>Feormgast Web Server</h1>"

      "<p><a href=\"/light/on\"><button class=\"button\">Turn ON</button></a></p>"
      "<p><a href=\"/light/on\"><button class=\"button button2\">ALSO ON</button></a></p>"

      "<h2>Chicken Coop Door</h2>"
      "<p><a href=\"/door/open\"><button class=\"button\">OPEN</button></a></p>"
      "<p><a href=\"/door/close\"><button class=\"button\">CLOSE</button></a></p>"

    "</body>"
  "</html>";
  

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });  
  server.on("/light/on", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
    digitalWrite(LED, HIGH);
  });
  server.on("/light/off", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
    digitalWrite(LED, LOW);
  });
  server.on("/door/open", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
    digitalWrite(LED, HIGH);
    motorOn = true;
    motorOnTime = millis();
    digitalWrite(OPENPIN, HIGH);
  });
  server.on("/door/close", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
    digitalWrite(LED, HIGH);
    motorOn = true;
    motorOnTime = millis();
    digitalWrite(CLOSEPIN, HIGH);
  });
  server.onNotFound( []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
}

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(10); }

  // Create Wifi Access Point
  Serial.print("Creating network ");
  Serial.println(SSID);
  Serial.println(PASSWORD);
  WiFi.softAP(SSID, PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(" at IP: ");
  Serial.println(IP);

  // httpUpdater.setup(&server);
  setupRoutes();
  server.begin();

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(OPENPIN, OUTPUT);
  digitalWrite(OPENPIN, LOW);
  pinMode(CLOSEPIN, OUTPUT);
  digitalWrite(CLOSEPIN, LOW);
}

void loop() {
  // WiFiClient client = server.available();

  if (motorOn && millis() - motorOnTime >= motorDuration) {
    digitalWrite(OPENPIN, LOW);
    digitalWrite(CLOSEPIN, LOW);
    digitalWrite(LED, LOW);
    motorOn = false;
  }

  server.handleClient();
  delay(2);
}