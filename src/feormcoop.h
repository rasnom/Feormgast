#define OPEN_PIN 23
#define CLOSE_PIN 22

class feormCoop {
    public:
        bool isOpen = false;
        bool isMotorOn = false;
        unsigned long motorOnTime = 0;
        const long motorDuration = 4500; // mS

        void openDoor();
        void closeDoor();
};