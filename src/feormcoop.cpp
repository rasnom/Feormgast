#include "feormcoop.h"

FeormCoop::FeormCoop() {
  pinMode(OPEN_PIN, OUTPUT);
  digitalWrite(OPEN_PIN, LOW);
  pinMode(CLOSE_PIN, OUTPUT);
  digitalWrite(CLOSE_PIN, LOW);
}

void FeormCoop::openDoor() {
  digitalWrite(OPEN_PIN, HIGH);
  isMotorOn = true;
  motorOnTime = millis();
  isOpen = true;
  Serial.println("Opening coop door");
  doorLog("open");
}

void FeormCoop::closeDoor() {
  digitalWrite(CLOSE_PIN, HIGH);
  isMotorOn = true;
  motorOnTime = millis();
  isOpen = false;
  Serial.println("Closing coop door");
  doorLog("close");
}

void FeormCoop::manageDoor() {
  int hour = coopClock.getHour(true);  // true to get 0-23 rather than 0-12

  if (isMotorOn) {
    if (millis() - motorOnTime >= motorDuration) {
      digitalWrite(OPEN_PIN, LOW);
      digitalWrite(CLOSE_PIN, LOW);
      isMotorOn = false;
    }
  } 
  else {
    if ((OPEN_TIME <= hour) && (hour < CLOSE_TIME)) {
      if (!isOpen) {
        Serial.print("It's ");
        Serial.print(hour);
        Serial.print(" which is after ");
        Serial.print(OPEN_TIME);
      Serial.print(" so ");
        openDoor();
      }
    } 
    else if (isOpen) {
      Serial.print("It's ");
      Serial.print(hour);
      Serial.print(" which is after ");
      Serial.print(CLOSE_TIME);
      Serial.print(" so ");
      closeDoor();
    }
  }
}

void FeormCoop::doorLog(String message) {
  File log;
  String logText = "";

  if (message == "open") {
    logText.concat("Door opened at ");
    logText.concat(coopClock.getDateTime());
  } 
  else if (message == "close") {
    logText.concat("Door closed at ");
    logText.concat(coopClock.getDateTime());
  }
  else {
    logText.concat("Door event ");
    logText.concat(message);
    logText.concat(" at ");
    logText.concat(coopClock.getDateTime());
  }

  log = SPIFFS.open("/doorlog.txt", FILE_APPEND);
  if(!log) {
    Serial.println("failed to log doorlog.txt from spiffs");
    return;
  } 
  else {
    log.println(logText);
  }
  log.close();

  FeormIO::sendNote(logText); 
}
