#include <SPIFFS.h>
#include <ESP32Time.h>
#include <Preferences.h>

class FeormIO {
    public:
        String wifiMode = "HUB";
        String unitName = "Default Hrothgar";
        ESP32Time rtc;
        Preferences preferences;

        void setup();
        String readFile(String);
        String serverIndex();  
        String firmwareUpdateForm();
        String javaScript();
        void switchWifiMode();
        void getPreferences();
};