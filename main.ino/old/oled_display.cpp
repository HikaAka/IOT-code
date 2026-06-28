#include <Arduino.h>

#include <Wire.h>

#include <Adafruit_GFX.h>

#include <Adafruit_SSD1306.h>

#include "oled_display.h"


//==================================================
// OLED設定
//==================================================

#define SCREEN_WIDTH 128

#define SCREEN_HEIGHT 64

#define OLED_RESET -1

#define OLED_ADDR 0x3C


//==================================================
// OLEDオブジェクト
//==================================================

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);


//==================================================
// ページ管理
//==================================================

int current_page = 0;

unsigned long last_switch = 0;

const unsigned long PAGE_INTERVAL = 3000;


//==================================================
// 初期化
//==================================================

bool oled_init() {

    Serial.println("OLED Start");


    /*
        OLED開始
    */
    if (!display.begin(SSD1306_SWITCHCAPVCC,
                       OLED_ADDR)) {

        Serial.println("OLED Failed");

        return false;
    }


    /*
        起動画面
    */
    display.clearDisplay();

    display.setTextSize(2);

    display.setTextColor(WHITE);

    display.setCursor(0, 10);

    display.println("START");

    display.display();

    delay(1500);

    return true;
}


//==================================================
// 更新
//==================================================

void oled_update(float pressure,
                 uint16_t co2,
                 float temperature,
                 float humidity) {

    //----------------------------------------------
    // ページ切替
    //----------------------------------------------

    if (millis() - last_switch > PAGE_INTERVAL) {

        current_page++;

        if (current_page > 2) {

            current_page = 0;
        }

        last_switch = millis();
    }


    //----------------------------------------------
    // 画面クリア
    //----------------------------------------------

    display.clearDisplay();

    display.setTextColor(WHITE);


    //----------------------------------------------
    // PAGE 0
    // CO2
    //----------------------------------------------

    if (current_page == 0) {

        display.setTextSize(2);

        display.setCursor(0, 0);

        display.println("CO2");

        display.setTextSize(3);

        display.setCursor(0, 28);

        display.print(co2);

        display.println("ppm");
    }


    //----------------------------------------------
    // PAGE 1
    // 温湿度
    //----------------------------------------------

    else if (current_page == 1) {

        display.setTextSize(2);

        display.setCursor(0, 0);

        display.print("Temp");

        display.setCursor(0, 32);

        display.print(temperature, 1);

        display.println("C");


        display.setCursor(70, 32);

        display.print(humidity, 0);

        display.println("%");
    }


    //----------------------------------------------
    // PAGE 2
    // 気圧
    //----------------------------------------------

    else if (current_page == 2) {

        display.setTextSize(2);

        display.setCursor(0, 0);

        display.println("Pressure");

        display.setTextSize(2);

        display.setCursor(0, 32);

        display.print(pressure, 1);

        display.println("hPa");
    }


    //----------------------------------------------
    // OLED反映
    //----------------------------------------------

    display.display();
}