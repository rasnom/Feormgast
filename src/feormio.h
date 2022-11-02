#include <SPIFFS.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <WiFi.h>
#include <secrets.h>
#include <esp_now.h>

// const uint8_t BROADCAST_ALL[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, }

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
        static void receiveData(const uint8_t *mac, const uint8_t *data, int length);

    private:
        const char *SSID = "Feormgast";
        const char *PASSWORD = AP_WIFI_PASSWORD;
        
        void setupWiFi();
        // void dataSent(const uint8_t *mac, esp_now_send_status_t status);
};