#include "feormio.h"

void FeormIO::setup() {
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed to load");
  }
  getPreferences();
  setupWiFi();
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW failed to start");
    return;
  }
  esp_now_register_recv_cb(receiveData);
}

void FeormIO::getPreferences(){
  preferences.begin("feormgast", false);

  if (preferences.isKey("unitName")) {
    unitName = preferences.getString("unitName");
  }

  // If the clock has not been set, default to HUB
  if (!preferences.isKey("wifiMode") || rtc.getYear() < 2022) {
    wifiMode = "HUB";
    preferences.putString("wifiMode", "HUB");
  } 
  else {
    wifiMode = preferences.getString("wifiMode");
  }
  preferences.end();
}

String FeormIO::readFile(String fileName) {
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

String FeormIO::serverIndex() {
  String indexHTML = "";
 
  indexHTML = readFile("/index.html");
  indexHTML.replace("%UNIT_NAME%", unitName);
  indexHTML.replace("%LOCAL_TIME%", rtc.getTime());
  indexHTML.replace("%WIFI_MODE%", wifiMode);
 
  return indexHTML;
}

String FeormIO::firmwareUpdateForm() {
  String formHtml = 
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'><input type='submit' value='Update'>"
    "</form>";
  return formHtml;
}

String FeormIO::javaScript() {
  String jscriptCode = ""; 
  jscriptCode = readFile("/script.js");
  return jscriptCode;
}

void FeormIO::switchWifiMode() {
  if (wifiMode == "HUB") {
    wifiMode = "NODE";
  } else {
    wifiMode = "HUB";
  }
  preferences.begin("feormgast", false);
  preferences.putString("wifiMode", wifiMode);
  preferences.end();
}

void FeormIO::setupWiFi() {
  unsigned long connectStartTime;

  // Create or Join Wifi Network
  Serial.println(SSID);
  Serial.println(PASSWORD);
  Serial.print("Mac Address : ");
  Serial.println(WiFi.macAddress());
  if (wifiMode == "HUB") {
    Serial.print("Creating Feormgast network ");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(SSID, PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    Serial.print(" at IP: ");
    Serial.println(IP);
    // Serial.print("Hub joining house network");
    // WiFi.begin(HOUSE_WIFI_SSID, HOUSE_WIFI_PASSWORD); // testing inside
    // WiFi.begin(SSID, PASSWORD); // deployed outside
  } 
  else { // "NODE"
    // Serial.print("Node joining house network");
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(HOUSE_WIFI_SSID, HOUSE_WIFI_PASSWORD);
    // connectStartTime = millis();
    // while (WiFi.status() != WL_CONNECTED) {
    //   if (millis() > connectStartTime + wifiTimeoutTime) {
    //     Serial.println("Wifi connection timed out");
    //     break;
    //   }
    //   Serial.print('.');
    //   delay(1000);
    // }
    // Serial.print(" at IP ");
    // Serial.println(WiFi.localIP());
  }
}

void FeormIO::receiveData(const uint8_t *mac, const uint8_t *data, int length) {
  Serial.print(length);
  Serial.print(" bytes received from ");
  Serial.println(*mac);
  Serial.println(*data);
}