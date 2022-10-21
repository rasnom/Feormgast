#include <Arduino.h>
#include <WiFi.h>
#include <Webserver.h>
#include <Update.h>
#include <ESP32Time.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include "secrets.h"
#include "feormcoop.h"

// #define LED 2
#define OPEN_PIN 23
#define CLOSE_PIN 22
#define OPEN_TIME 6
#define CLOSE_TIME 19
#define uS_TO_mS 1000
#define AWAKE_TIME 100000 // mS milliseconds
#define SLEEP_TIME 1000000 // mS

const char *SSID = "Feormgast";
const char *PASSWORD = AP_WIFI_PASSWORD;
unsigned long wakeTime = millis();
const long wifiTimeoutTime = 2000; // mS 

WebServer server(80);
String header;
ESP32Time rtc;
Preferences preferences;
String unitName;
String wifiMode;
feormCoop coop;

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

void doorLog(String message) {
  File log;

  log = SPIFFS.open("/doorlog.txt", FILE_APPEND);
  if(!log) {
    Serial.println("failed to log doorlog.txt from spiffs");
    return;
  }
  if (message == "open") {
    log.print("Door opened at ");
    log.println(rtc.getDateTime());
  } 
  else if (message == "close") {
    log.print("Door closed at ");
    log.println(rtc.getDateTime());
  }
  else {
    log.print("Door event ");
    log.print(message);
    log.print(" at ");
    log.println(rtc.getDateTime());
  }
  log.close();
}

void openDoor() {
  coop.isMotorOn = true;
  coop.motorOnTime = millis();
  digitalWrite(OPEN_PIN, HIGH);
  Serial.println("Opening coop door");
  doorLog("open");
  coop.isOpen = true;
}

void closeDoor() {
  coop.isMotorOn = true;
  coop.motorOnTime = millis();
  digitalWrite(CLOSE_PIN, HIGH);
  Serial.println("Closing coop door");
  doorLog("close");
  coop.isOpen = false;
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

  server.sendHeader("Connection", "close");
  server.send(200, "text/html", serverIndex());
  clientMillis = server.arg("clientMillis");
  offset = 60 * atol(server.arg("clientOffset").c_str()); // UTC offset
  millis = atoll(clientMillis.c_str()) / 1000 - offset;
  rtc.setTime(millis);
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
    openDoor();
  });
  server.on("/close", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex());
    closeDoor();
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
    server.enableDelay(false);
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "Restarting");
    switchWifiMode(); 
    delay(1); // prevents the ESP from restarting before the server finishes
    ESP.restart();
  });
  server.on("/log", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", readFile("/doorlog.txt"));
  });
  server.onNotFound( []() {
    server.sendHeader("Connection", "close");
    server.send(404, "text/plain", "Unknown Request");
    Serial.println("route not found");
    Serial.println(server.uri());
  });
}

void manageDoor() {
  int hour = rtc.getHour();

  if ((OPEN_TIME <= hour) && (hour < CLOSE_TIME)) {
    if (!coop.isOpen) {
      openDoor();
    }
  } else 
    if (coop.isOpen) {
      closeDoor();
    }
}

void setupWiFi() {
  // Create or Join Wifi Network
  Serial.println(SSID);
  Serial.println(PASSWORD);
  if (wifiMode == "HUB") {
    Serial.print("Creating Feormgast network ");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(SSID, PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    Serial.print(" at IP: ");
    Serial.println(IP);
    Serial.print("Hub joining house network");
    WiFi.begin(HOUSE_WIFI_SSID, HOUSE_WIFI_PASSWORD); // testing inside
    // WiFi.begin(SSID, PASSWORD); // deployed outside
  } 
  else { // "NODE"
    Serial.print("Node joining house network");
    WiFi.mode(WIFI_STA);
    WiFi.begin(HOUSE_WIFI_SSID, HOUSE_WIFI_PASSWORD);
  }
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print(" at IP ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(10); };

  Serial.print("Woken by ");
  Serial.println(esp_sleep_get_wakeup_cause());

  preferences.begin("feormgast", false);
  if (preferences.isKey("unitName")) {
    unitName = preferences.getString("unitName");
  }
  else {
    unitName = "Hrothgar 1.1";
    preferences.putString("unitName", unitName);
  }
  if (preferences.isKey("wifiMode")) {
    wifiMode = preferences.getString("wifiMode");
  }
  else {
    wifiMode = "HUB";
    preferences.putString("wifiMode", wifiMode);
  }
  preferences.end();

  setupWiFi();
  setupRoutes();
  server.begin();

  // pinMode(LED, OUTPUT);
  // digitalWrite(LED, LOW);
  pinMode(OPEN_PIN, OUTPUT);
  digitalWrite(OPEN_PIN, LOW);
  pinMode(CLOSE_PIN, OUTPUT);
  digitalWrite(CLOSE_PIN, LOW);

  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed to load");
    return;
  }
}

void loop() {
  if (coop.isMotorOn) {
    if (millis() - coop.motorOnTime >= coop.motorDuration) {
      digitalWrite(OPEN_PIN, LOW);
      digitalWrite(CLOSE_PIN, LOW);
      coop.isMotorOn = false;
    }
  } 
  else if (wifiMode == "NODE" && millis() - wakeTime > AWAKE_TIME) {
    esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_mS);
    Serial.println("Going to sleep");
    Serial.flush();
    esp_deep_sleep_start();
  }

  manageDoor();
  server.handleClient();
  delay(2);
}