#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_SHT4x.h>

#include "sht41_sensor.h"

Adafruit_SHT4x sht4;

//==================================================
// 初期化
//==================================================

bool sht41_init() {

    if (!sht4.begin()) {

        Serial.println("SHT41 not found");

        return false;
    }

    Serial.println("SHT41 OK");

    return true;
}


//==================================================
// 読み取り
//==================================================

bool sht41_read(float* temp,
                float* hum) {

    sensors_event_t humidity;
    sensors_event_t temperature;

    if (!sht4.getEvent(&humidity,
                       &temperature)) {

        return false;
    }

    *temp = temperature.temperature;

    *hum = humidity.relative_humidity;

    return true;
}