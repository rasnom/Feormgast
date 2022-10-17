#include <Arduino.h>
#include <WiFi.h>
#include <Webserver.h>
#include <Update.h>
#include <ESP32Time.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include "secrets.h"

// #define LED 2
#define OPENPIN 23
#define CLOSEPIN 22
#define OPENTIME 6
#define CLOSETIME 20

const char *SSID = "Feormgast";
const char *PASSWORD = AP_WIFI_PASSWORD;
bool motorOn = false;
bool doorOpen = false;  
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long motorOnTime = 0;
const long wifiTimeoutTime = 2000; // milliseconds
const long motorDuration = 4500;

WebServer server(80);
String header;
ESP32Time rtc;
Preferences preferences;
String unitName;
String wifiMode;

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
  indexHTML.replace("%UNIT_NAME%", unitName);
  indexHTML.replace("%LOCAL_TIME%", rtc.getTime());
  indexHTML.replace("%WIFI_MODE%", wifiMode);
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

void updateFirmware() {
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
}

void switchWifiMode() {
  if (wifiMode == "HUB") {
    wifiMode = "NODE";
  } else {
    wifiMode = "HUB";
  }
  preferences.begin("feormgast", false);
  preferences.putString("wifiMode", wifiMode);
  preferences.end();
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
  }, updateFirmware);
  server.on("/script.js", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/javascript", javaScript());
  });
  server.on("/clocksync", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex());
    Serial.println(server.args());
    clockSync();
  });
  server.on("/switchWifiMode", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", "Restarting");
    switchWifiMode();
    ESP.restart();
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

  preferences.begin("feormgast", true);
  unitName = preferences.getString("unitName", "default Hrothgar");
  wifiMode = preferences.getString("wifiMode", "HUB");
  preferences.end();

  // Create or Join Wifi Network
  Serial.println(SSID);
  Serial.println(PASSWORD);
  if (wifiMode == "HUB") {
    Serial.print("Creating network ");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    Serial.print(" at IP: ");
    Serial.println(IP);
  } else { // "NODE"
    Serial.print("Joining network");
    WiFi.mode(WIFI_STA);
    WiFi.begin(HOUSE_WIFI_SSID, HOUSE_WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
    Serial.print(" at IP ");
    Serial.println(WiFi.localIP());
  }

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