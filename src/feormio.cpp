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
