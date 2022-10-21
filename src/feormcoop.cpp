#include <ESP32Time.h>
#include "feormcoop.h"

void feormCoop::openDoor() {
  isMotorOn = true;
  motorOnTime = millis();
  digitalWrite(OPEN_PIN, HIGH);
  Serial.println("Opening coop door");
//   doorLog("open");
  isOpen = true;
}


void feormCoop::closeDoor() {
  isMotorOn = true;
  motorOnTime = millis();
  digitalWrite(CLOSE_PIN, HIGH);
  Serial.println("Closing coop door");
//   doorLog("close");
  isOpen = false;
}
