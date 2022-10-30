#include "feormio.h"

void FeormIO::setup() {
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed to load");
  }
  getPreferences();
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