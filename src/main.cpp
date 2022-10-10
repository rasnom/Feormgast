#include <Arduino.h>
#include <WiFi.h>
#include <Webserver.h>
#include <Update.h>
#include <ESP32Time.h>
#include <SPIFFS.h>
#include "secrets.h"

// #define LED 2
#define OPENPIN 22
#define CLOSEPIN 23
#define OPENTIME 7
#define CLOSETIME 20

const char *SSID = "Feormgast";
const char *PASSWORD = AP_WIFI_PASSWORD;
bool motorOn = false;
bool doorOpen = false;  
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long motorOnTime = 0;
const long wifiTimeoutTime = 2000; // milliseconds
const long motorDuration = 5000;

WebServer server(80);
String header;
ESP32Time rtc;

String readFile(String fileName) {
  File file;
  String fileText;

  file = SPIFFS.open(fileName);
  if(!file) {
    Serial.print("failed to load ");
    Serial.print(fileName);
    Serial.println(" from SPIFFS");
  }
  fileText = file.readString();

  return fileText; 
}

String serverIndex() {
  String indexHTML = "";
  indexHTML = readFile("/index.html");
  indexHTML.replace("%LOCAL_TIME%", rtc.getTime());
  return indexHTML;
}
  
String updateForm() {
  String formHtml = 
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'><input type='submit' value='Update'>"
    "</form>";
  return formHtml;
}

String javaScript() {
  String jscriptCode = ""; 
  jscriptCode = readFile("/script.js");
  return jscriptCode;
}

void clockSync() {
  String clientMillis;
  long long millis;
  long offset; 

  clientMillis = server.arg("clientMillis");
  offset = 60 * atol(server.arg("clientOffset").c_str()); // UTC offset
  millis = atoll(clientMillis.c_str()) / 1000 - offset;
  rtc.setTime(millis);
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", serverIndex());
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex());
  });  
  server.on("/open", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex());
    motorOn = true;
    motorOnTime = millis();
    digitalWrite(OPENPIN, HIGH);
  });
  server.on("/close", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex());
    motorOn = true;
    motorOnTime = millis();
    digitalWrite(CLOSEPIN, HIGH);
  });
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateForm());
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
    server.send(200, "text/javascript", javaScript());
  });
  server.on("/clocksync", HTTP_GET, []() {
    Serial.println(server.args());
    clockSync();
  });
  server.onNotFound( []() {
    server.sendHeader("Connection", "close");
    server.send(404, "text/plain", "Unknown Request");
    Serial.println("route not found");
    Serial.println(server.uri());
  });
}

void openDoor() {
  motorOn = true;
  motorOnTime = millis();
  digitalWrite(OPENPIN, HIGH);
  Serial.println("Opening coop door");
  doorOpen = true;
}

void closeDoor() {
  motorOn = true;
  motorOnTime = millis();
  digitalWrite(CLOSEPIN, HIGH);
  Serial.println("Closing coop door");
  doorOpen = false;
}

void manageDoor() {
  int hour = rtc.getHour();

  if ((OPENTIME <= hour) && (hour < CLOSETIME)) {
    if (!doorOpen) {
      openDoor();
    }
  } else 
    if (doorOpen) {
      closeDoor();
    }
}

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(10); };

  // Create Wifi Access Point
  Serial.print("Creating network ");
  Serial.println(SSID);
  Serial.println(PASSWORD);
  WiFi.softAP(SSID, PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(" at IP: ");
  Serial.println(IP);
  Serial.print("host   ");

  setupRoutes();
  server.begin();

  // pinMode(LED, OUTPUT);
  // digitalWrite(LED, LOW);
  pinMode(OPENPIN, OUTPUT);
  digitalWrite(OPENPIN, LOW);
  pinMode(CLOSEPIN, OUTPUT);
  digitalWrite(CLOSEPIN, LOW);

  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed to load");
    return;
  }
}

void loop() {
  if (motorOn && millis() - motorOnTime >= motorDuration) {
    digitalWrite(OPENPIN, LOW);
    digitalWrite(CLOSEPIN, LOW);
    motorOn = false;
  }
  manageDoor();
  server.handleClient();
  delay(2);
}