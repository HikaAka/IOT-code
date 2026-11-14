#include <Arduino.h>

#include <WiFi.h>

#include "wifi_manager.h"
#include "secrets.h"




//==================================================
// WiFi接続
//==================================================

bool wifi_connect() {

    Serial.println("WiFi Connecting...");


    /*
        WiFi接続開始
    */
    WiFi.begin(WIFI_SSID,
               WIFI_PASSWORD);


    /*
        接続待機
    */
    int retry = 0;

    while (WiFi.status() != WL_CONNECTED) {

        delay(500);

        Serial.print(".");


        retry++;

        /*
            接続失敗タイムアウト
        */
        if (retry > 30) {

            Serial.println();

            Serial.println("WiFi Failed");

            return false;
        }
    }

    Serial.println();

    Serial.println("WiFi Connected");

    Serial.print("IP: ");

    Serial.println(WiFi.localIP());

    return true;
}