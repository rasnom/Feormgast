#include "feormio.h"

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

