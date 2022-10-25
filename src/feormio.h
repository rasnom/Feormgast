#include <SPIFFS.h>
#include <ESP32Time.h>

class FeormIO {
    public:
        String wifiMode = "HUB";
        String unitName = "Default Hrothgar";
        ESP32Time rtc;

        String readFile(String);
        String serverIndex();  
        String firmwareUpdateForm();
        String javaScript();
};