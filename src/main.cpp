#include <Arduino.h>
#include <WiFi.h>
#include <Webserver.h>
#include <Update.h>
#include "secrets.h"

#define LED 2
#define OPENPIN 22
#define CLOSEPIN 23

const char *SSID = "Feormgast";
const char *PASSWORD = AP_WIFI_PASSWORD;

WebServer server(80);
String header;

bool motorOn = false;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long motorOnTime = 0;
const long wifiTimeoutTime = 2000; // milliseconds
const long motorDuration = 5000;

const char *serverIndex = 
  "<!DOCTYPE html>"
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
    "<div id = \"digital-clock\"> </div>"

    "<p><a href=\"/light/on\"><button class=\"button\">Turn ON</button></a></p>"
    "<p><a href=\"/light/on\"><button class=\"button button2\">ALSO ON</button></a></p>"

    "<h2>Chicken Coop Door</h2>"
    "<p><a href=\"/door/open\"><button class=\"button\">OPEN</button></a></p>"
    "<p><a href=\"/door/close\"><button class=\"button\">CLOSE</button></a></p>"

    "<script src = \"script.js\"> </script>"
  "</body>";
  
const char *updateForm = 
  "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='update'><input type='submit' value='Update'>"
  "</form>";

const char *javaScript = 
  "function Time() {\n"
      "// Creating object of the Date class\n"
      "var date = new Date();\n"
      "// Get current hour\n"
      "var hour = date.getHours();\n"
      "// Get current minute\n"
      "var minute = date.getMinutes();\n"
      "// Get current second\n"
      "var second = date.getSeconds();\n"
      "// Variable to store AM / PM\n"
      "var period = \"\";\n"
      "// Assigning AM / PM according to the current hour\n"
      "if (hour >= 12) {\n"
      "period = \"PM\";\n"
      "} else {\n"
      "period = \"AM\";\n"
      "}\n"
      "// Converting the hour in 12-hour format\n"
      "if (hour == 0) {\n"
      "hour = 12;\n"
      "} else {\n"
      "if (hour > 12) {\n"
      "hour = hour - 12;\n"
      "}\n"
      "}\n"
      "// Updating hour, minute, and second\n"
      "// if they are less than 10\n"
      "hour = update(hour);\n"
      "minute = update(minute);\n"
      "second = update(second);\n"
      "// Adding time elements to the div\n"
      "document.getElementById(\"digital-clock\").innerText = hour + \" : \" + minute + \" : \" + second + \" \" + period;\n"
      "// Set Timer to 1 sec (1000 ms)\n"
      "setTimeout(Time, 1000);\n"
  "}\n"
      "// Function to update time elements if they are less than 10\n"
      "// Append 0 before time elements if they are less than 10\n"
  "function update(t) {\n"
      "if (t < 10) {\n"
      "return \"0\" + t;\n"
      "}\n"
      "else {\n"
      "return t;\n"
      "}\n"
  "}\n"
  "Time();\n"
;

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
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateForm);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin()) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    } else {
      Serial.printf("Update Failed: status is %d\n", upload.status);
    }
  });
  server.on("/script.js", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/javascript", javaScript);
  });
  server.onNotFound( []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "Unknown Request");
    Serial.println("route not found");
    Serial.println(server.uri());
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
  if (motorOn && millis() - motorOnTime >= motorDuration) {
    digitalWrite(OPENPIN, LOW);
    digitalWrite(CLOSEPIN, LOW);
    digitalWrite(LED, LOW);
    motorOn = false;
  }
  server.handleClient();
  delay(2);
}