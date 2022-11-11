
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <averager.h>

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
float floatValue;
bool signupOK = false;
averager<int, 10> voltage;

constexpr char WIFI_SSID[] = "HUAWEIMario";
constexpr char WIFI_PASS[] = "";

void initWiFi() {

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.printf("Conectando a rede %s .", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(200); }
    Serial.println(" ok");

    IPAddress ip = WiFi.localIP();

    Serial.printf("SSID: %s\n", WIFI_SSID);
    Serial.printf("Canal: %u\n", WiFi.channel());
    Serial.printf("IP: %u.%u.%u.%u\n", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, ip >> 24);

  

}


void onReceive(const uint8_t *mac_addr, const uint8_t *data, int len) {

    Serial.printf("Recebeu: %3u de %02x:%02x:%02x:%02x:%02x:%02x\n",
        (uint8_t) *data,
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]
    );

    Serial.println(*data);

    voltage.append(*data);




}

void initEspNow() {

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP NOW falhou ao iniciar");
        while (1);
    }

    esp_now_register_recv_cb(onReceive);
}


void setup() {
    Serial.begin(115200);
    delay(500);

    initWiFi();
    initEspNow();

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; // addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void loop(){
     if(Firebase.ready()){
       if (Firebase.RTDB.pushInt(&fbdo, "painel/teste", voltage.getAverage())){
      Serial.println("Caminho do dado: " + fbdo.dataPath());
      Serial.println("Tipo: " + fbdo.dataType());
    }
    else {
      Serial.println("ERRO:: " + fbdo.errorReason());
    }
    }
}
