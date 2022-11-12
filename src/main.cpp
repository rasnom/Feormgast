#include <Arduino.h>
#include <Webserver.h>
#include <Update.h>
#include <ESP32Time.h>
#include "feormcoop.h"
#include "feormio.h"

// #define LED 2
#define uS_TO_mS 1000
#define AWAKE_TIME 100000 // mS milliseconds
#define SLEEP_TIME 1000000 // mS

unsigned long wakeTime = millis();

WebServer server(80);
ESP32Time rtc;
FeormCoop coop;
FeormIO comms;

void clockSync() {
  String clientMillis;
  long long millis;
  long offset; 

  server.sendHeader("Connection", "close");
  server.send(200, "text/html", comms.serverIndex());
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

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", comms.serverIndex());
  });  
  server.on("/open", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", comms.serverIndex());
    coop.openDoor();
  });
  server.on("/close", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", comms.serverIndex());
    coop.closeDoor();
  });
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", comms.firmwareUpdateForm());
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, updateFirmware);
  server.on("/script.js", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/javascript", comms.javaScript());
  });
  server.on("/clocksync", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    clockSync();
    server.send(200, "text/html", comms.serverIndex());
  });
  server.on("/switchWifiMode", HTTP_GET, []() {
    server.enableDelay(false);
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "Restarting");
    comms.switchWifiMode(); 
    delay(100); // prevents the ESP from restarting before the server finishes
    ESP.restart();
  });
  server.on("/log", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", comms.readFile("/doorlog.txt"));
  });
  server.onNotFound( []() {
    server.sendHeader("Connection", "close");
    server.send(404, "text/plain", "Unknown Request");
    Serial.print("route not found: ");
    Serial.println(server.uri());
  });
}

void maybeSleep() {
  if (comms.wifiMode == "NODE" && millis() - wakeTime > AWAKE_TIME) {
    esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_mS);
    Serial.println("Going to sleep");
    Serial.flush();
    esp_deep_sleep_start();
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(10); };

  Serial.print("Woken by ");
  Serial.println(esp_sleep_get_wakeup_cause());

  comms.setup();
  Serial.print("WIfi mode - ");
  Serial.println(comms.wifiMode);
  if (comms.wifiMode == "HUB") {
    setupRoutes();
    server.begin();
  }
  
  // pinMode(LED, OUTPUT);
  // digitalWrite(LED, LOW);
  pinMode(OPEN_PIN, OUTPUT);
  digitalWrite(OPEN_PIN, LOW);
  pinMode(CLOSE_PIN, OUTPUT);
  digitalWrite(CLOSE_PIN, LOW);

  // Close coop door first thing, so we know it starts closed
  // It will open again right away if it is daytime.
  Serial.print("Starting up so ");
  coop.closeDoor();
}

void loop() {
  coop.manageDoor();
  server.handleClient();
  if (comms.wifiMode == "NODE") {
    FeormIO::sendNote("a note from main loop");
  }
  delay(400);
  maybeSleep();
}