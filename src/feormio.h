#include <SPIFFS.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <WiFi.h>
#include <secrets.h>
// #include <esp_now.h>

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

    private:
        const char *SSID = "Feormgast";
        const char *PASSWORD = AP_WIFI_PASSWORD;
        
        void setupWiFi();
};