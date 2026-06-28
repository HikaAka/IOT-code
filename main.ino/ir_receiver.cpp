#include <Arduino.h>

#include <IRremoteESP8266.h>

#include <IRrecv.h>

#include <IRutils.h>

#include "ir_receiver.h"


//==================================================
// IR設定
//==================================================

const uint16_t IR_PIN = 14;

const uint16_t CAPTURE_BUFFER_SIZE = 400;

const uint8_t TIMEOUT = 50;


//==================================================
// IR受信オブジェクト
//==================================================

IRrecv irrecv(
    IR_PIN,
    CAPTURE_BUFFER_SIZE,
    TIMEOUT,
    true
);


//==================================================
// 受信結果
//==================================================

decode_results results;


//==================================================
// 初期化
//==================================================

bool ir_receiver_init() {

    Serial.println("IR Receiver Start");


    /*
        赤外線受信開始
    */
    irrecv.enableIRIn();


    Serial.println("IR Receiver OK");

    return true;
}


//==================================================
// 更新
//==================================================

void ir_receiver_update() {

    /*
        信号受信確認
    */
    if (irrecv.decode(&results)) {

        Serial.println();
        Serial.println("================================");

        Serial.println("IR Signal Received");

        Serial.println("================================");


        //------------------------------------------
        // 基本解析表示
        //------------------------------------------

        Serial.print(
            resultToHumanReadableBasic(
                &results
            )
        );


        //------------------------------------------
        // RAWコード表示
        //------------------------------------------

        Serial.println();

        Serial.println("--- RAW ---");

        Serial.println(
            resultToSourceCode(
                &results
            )
        );


        //------------------------------------------
        // 次の受信準備
        //------------------------------------------

        irrecv.resume();
    }
}