#include <Wire.h>

#include <Arduino.h>

#include "i2c_manager.h"

void i2c_init() {
    //------------------------------------------
    // I2C初期化
    //------------------------------------------

    Wire.end();

    delay(100);

    Wire.begin(21, 22);

    Serial.println("I2C READY");

    delay(100);
    // SDA, SCL
    // Wire.begin(21, 22);

    // 安定重視
    Wire.setClock(10000);
}