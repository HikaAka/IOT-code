#include <Arduino.h>

#include <Wire.h>

#include <SparkFun_SCD4x_Arduino_Library.h>

#include "scd40_sensor.h"


//==================================================
// SCD40オブジェクト
//==================================================

SCD4x scd40;


//==================================================
// 初期化
//==================================================

bool scd40_init() {

    Serial.println("SCD40 Start");


    /*
        I2C接続開始

        Wire.begin() は不要

        既に i2c_init() で
        初期化済みだから
    */
    if (!scd40.begin()) {

        Serial.println("SCD40 not found");

        return false;
    }


    /*
        周期測定開始

        SCD40は開始コマンドが必要
    */
    if (!scd40.startPeriodicMeasurement()) {

        Serial.println("Measurement start failed");

        return false;
    }

    Serial.println("SCD40 OK");

    return true;
}


//==================================================
// 読み取り
//==================================================

bool scd40_read(uint16_t* co2,
                float* temperature,
                float* humidity) {

    /*
        新しいデータが来たか確認
    */



    bool ready = scd40.readMeasurement();




    /*
        データ未更新
    */
    if (!ready) {

        return false;
    }


    /*
        値取得
    */
    uint16_t co2_value =
        scd40.getCO2();

    float temp_value =
        scd40.getTemperature();

    float hum_value =
        scd40.getHumidity();


    /*
        異常値除外
    */
    if (co2_value < 300 ||
        co2_value > 10000) {

        return false;
    }


    /*
        呼び出し元へ返す
    */
    *co2 = co2_value;

    *temperature = temp_value;

    *humidity = hum_value;

    return true;
}

// void scd40_set_pressure(float pressure_hpa) {

//     uint16_t pressure =
//         (uint16_t)(pressure_hpa + 0.5);

//     scd40.setAmbientPressure(pressure);
// }