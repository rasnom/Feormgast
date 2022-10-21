#include <SPIFFS.h>
#include "feormcoop.h"

void FeormCoop::openDoor() {
  isMotorOn = true;
  motorOnTime = millis();
  digitalWrite(OPEN_PIN, HIGH);
  Serial.println("Opening coop door");
  doorLog("open");
  isOpen = true;
}

void FeormCoop::closeDoor() {
  isMotorOn = true;
  motorOnTime = millis();
  digitalWrite(CLOSE_PIN, HIGH);
  Serial.println("Closing coop door");
  doorLog("close");
  isOpen = false;
}

void FeormCoop::manageDoor() {
  int hour = coopClock.getHour();

  if ((OPEN_TIME <= hour) && (hour < CLOSE_TIME)) {
    if (isOpen) {
      openDoor();
    }
  } else 
    if (isOpen) {
      closeDoor();
    }

  if (isMotorOn) {
    if (millis() - motorOnTime >= motorDuration) {
      digitalWrite(OPEN_PIN, LOW);
      digitalWrite(CLOSE_PIN, LOW);
      isMotorOn = false;
    }
  } 

}

void FeormCoop::doorLog(String message) {
  File log;

  log = SPIFFS.open("/doorlog.txt", FILE_APPEND);
  if(!log) {
    Serial.println("failed to log doorlog.txt from spiffs");
    return;
  }
  if (message == "open") {
    log.print("Door opened at ");
    log.println(coopClock.getDateTime());
  } 
  else if (message == "close") {
    log.print("Door closed at ");
    log.println(coopClock.getDateTime());
  }
  else {
    log.print("Door event ");
    log.print(message);
    log.print(" at ");
    log.println(coopClock.getDateTime());
  }
  log.close();
}
