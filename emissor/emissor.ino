
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

constexpr char WIFI_SSID[] = "HUAWEIMario";

int32_t getWiFiChannel(const char *ssid) {

    if (int32_t n = WiFi.scanNetworks()) {
        for (uint8_t i=0; i<n; i++) {
            if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
                return WiFi.channel(i);
            }
        }
    }

    return 0;
}

void initWiFi() {

    WiFi.mode(WIFI_MODE_STA);

    int32_t channel = getWiFiChannel(WIFI_SSID);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
}
constexpr uint8_t ESP_NOW_RECEIVER[] = { 0xAC, 0x67, 0xB2, 0x3C, 0x37, 0x90 }; // AC:67:B2:3C:37:90

esp_now_peer_info_t peerInfo;

void initEspNow() {

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP NOW não inicializou");
        while (1);
    }

    memcpy(peerInfo.peer_addr, ESP_NOW_RECEIVER, 6);
    peerInfo.ifidx   = ESP_IF_WIFI_STA;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("ESP NOW pairing failure");
        while (1);
    }
}

void setup() {
    Serial.begin(115200);

    initWiFi();
    initEspNow();
}


uint32_t last;

void loop() {

    if (millis() - last > 2000) {
        
        uint8_t data = random(1, 256);
        esp_now_send(ESP_NOW_RECEIVER, (uint8_t *) &data, sizeof(uint8_t));
        
        Serial.printf("%3u CANAL: %u\n", data, WiFi.channel());

        last = millis();
    }
}
