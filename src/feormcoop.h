#include <ESP32Time.h>

#define OPEN_PIN 23
#define CLOSE_PIN 22
#define OPEN_TIME 6
#define CLOSE_TIME 19

class FeormCoop {
    public:
        bool isOpen = false;
        bool isMotorOn = false;
        unsigned long motorOnTime = 0;
        const long motorDuration = 4000; // mS
        ESP32Time coopClock;

        void openDoor();
        void closeDoor();
        void doorLog(String);
        void manageDoor();
};