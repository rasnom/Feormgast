#ifndef FEORMIO_H_
#define FEORMIO_H_

#include <SPIFFS.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <WiFi.h>
#include <secrets.h>
#include <esp_now.h>

class FeormIO {
    public:
        const long wifiTimeoutTime = 7000; // mS 

        String wifiMode = "HUB";
        String unitName = "Default Hrothgar";
        ESP32Time rtc;
        Preferences preferences;
        // uint8_t SEND_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


        void setup();
        String readFile(String);
        String serverIndex();  
        String firmwareUpdateForm();
        String javaScript();
        void switchWifiMode();
        void getPreferences();
        static void receiveData(const uint8_t *mac, const uint8_t *data, int length);
        static void dataSent(const uint8_t *mac, esp_now_send_status_t status);
        static void sendNote(String str);
        static void writeLog(String message);
        void setupESPNow();

    private:
        const char *SSID = "Feormgast";
        const char *PASSWORD = AP_WIFI_PASSWORD;
        
        void setupWiFi();
};

struct espMessage {
    char text[32];
};

#endif // FEORMIO_H_