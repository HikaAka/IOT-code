#include <Arduino.h>

#include <WiFi.h>

#include <Ambient.h>

#include "ambient_client.h"


//==================================================
// Ambient設定
//==================================================

unsigned int CHANNEL_ID =
    100790;

const char* WRITE_KEY =
    "76aa0083c6ba488a";


//==================================================
// Ambientオブジェクト
//==================================================

WiFiClient client;

Ambient ambient;


//==================================================
// 初期化
//==================================================

bool ambient_init() {

    /*
        Ambient開始
    */
    ambient.begin(CHANNEL_ID,
                  WRITE_KEY,
                  &client);

    Serial.println("Ambient Ready");

    return true;
}


//==================================================
// データ送信
//==================================================
static unsigned long lastSend = 0;

bool ambient_send(float pressure,
                  uint16_t co2,
                  float temperature,
                  float humidity,
                  float sht_temperature,
                  float sht_humidity,
                  float absolute_humidity,
                  float dew_point                  
                  ) {
    if (millis() - lastSend < 30000) {

        return true;
    }
    lastSend = millis();

    /*
        d1 = 気圧
    */
    ambient.set(1, pressure);

    /*
        d2 = CO2
    */
    ambient.set(2, co2);

    /*
        d3 = 温度
    */
    ambient.set(3, temperature);

    /*
        d4 = 湿度
    */
    ambient.set(4, humidity);
    
    /*
        d5 = 温度
    */
    ambient.set(5, sht_temperature);

    /*
        d6 = 湿度
    */
    ambient.set(6, sht_humidity);
    /*
        d7 = 絶対湿度
    */
    ambient.set(7, absolute_humidity);

    /*
        d8 = 露店温度
    */
    ambient.set(8, dew_point);

    /*
        送信
    */
    bool result =
        ambient.send();


    if (result) {

        Serial.println("Ambient Send OK");

    } else {

        Serial.println("Ambient Send Failed");
    }

    return result;
}