/*
    Arduino基本機能
    Serial, delay, uint8_t などを使うため
*/
#include <Arduino.h>

/*
    I2C通信用ライブラリ
    ESP32からLPS25HBへ通信するため
*/
#include <Wire.h>

/*
    自分で作ったヘッダファイル
    lps25_init()
    lps25_read()
    などの宣言が入っている
*/
#include "lps25_sensor.h"


//==================================================
// LPS25HBのI2Cアドレス
//==================================================

/*
    0x5C は LPS25HB の I2Cアドレス

    SA0ピンをGNDにした場合：
    0x5C

    SA0ピンをVDDにした場合：
    0x5D
*/
#define LPS25_ADDR 0x5C


//==================================================
// レジスタアドレス定義
//==================================================

/*
    WHO_AM_Iレジスタ

    「本当にLPS25HBが接続されているか」
    を確認するためのレジスタ

    正常なら 0xBD が返る
*/
#define WHO_AM_I 0x0F


/*
    CTRL_REG1

    センサの動作設定レジスタ

    Power ON/OFF
    出力周期
    Low-pass filter

    などを設定する
*/
#define CTRL_REG1 0x20


/*
    気圧データ用レジスタ

    24bitデータを
    3つの8bitレジスタに分割している

    XL = 下位8bit
    L  = 中位8bit
    H  = 上位8bit
*/
#define PRESS_OUT_XL 0x28
#define PRESS_OUT_L  0x29
#define PRESS_OUT_H  0x2A


/*
    温度データ用レジスタ

    16bitデータを
    2つの8bitレジスタに分割
*/
#define TEMP_OUT_L 0x2B
#define TEMP_OUT_H 0x2C


/*
    RPDSレジスタ

    気圧補正用オフセットレジスタ

    センサ個体差を補正できる

    電源OFFで消えるので
    起動時に毎回設定必要
*/
#define RPDS_L 0x39
#define RPDS_H 0x3A


//==================================================
// レジスタ書き込み
//==================================================

/*
    指定レジスタへ値を書き込む関数

    例：
    writeRegister(0x20, 0x90)

    ↓

    CTRL_REG1 に 0x90 を書く
*/
void writeRegister(uint8_t reg,
                   uint8_t value) {

    /*
        I2C通信開始

        LPS25HBへ送信開始
    */
    Wire.beginTransmission(LPS25_ADDR);

    /*
        最初にレジスタ番号送信

        「どこへ書くか」
    */
    Wire.write(reg);

    /*
        次に書き込む値送信

        「何を書くか」
    */
    Wire.write(value);

    /*
        通信終了
    */
    Wire.endTransmission();
}


//==================================================
// レジスタ読み取り
//==================================================

/*
    指定レジスタを1byte読む

    例：
    readRegister(0x0F)

    ↓

    WHO_AM_I読み取り
*/
uint8_t readRegister(uint8_t reg) {

    /*
        通信開始
    */
    Wire.beginTransmission(LPS25_ADDR);

    /*
        読みたいレジスタ番号送信
    */
    Wire.write(reg);

    /*
        false を指定すると
        Repeated Startになる

        一度通信を切らずに
        そのまま読み取りへ移行

        I2Cではよく使う
    */
    Wire.endTransmission(false);

    /*
        1byte読み取り要求
    */
    Wire.requestFrom(LPS25_ADDR, 1);

    /*
        読み取った1byte返す
    */
    return Wire.read();
}


//==================================================
// 気圧補正設定
//==================================================

/*
    センサ個体差補正

    例：
    実際より +5hPa 高く出る

    ↓

    setPressureOffset(5)

    を設定すると
    以後自動補正される
*/
void setPressureOffset(float offset_hpa) {

    /*
        LPS25HB内部形式へ変換

        データシート式：
        RPDS = hPa × (4096 / 256)
    */
    int16_t rpds =
        offset_hpa * (4096.0 / 256.0);

    /*
        デバッグ表示
    */
    Serial.print("RPDS = ");
    Serial.println(rpds);

    /*
        下位8bit取り出し
    */
    uint8_t low =
        rpds & 0xFF;

    /*
        上位8bit取り出し
    */
    uint8_t high =
        (rpds >> 8) & 0xFF;

    /*
        RPDSレジスタへ書き込み
    */
    writeRegister(RPDS_L, low);

    writeRegister(RPDS_H, high);
}


//==================================================
// 初期化
//==================================================

bool lps25_init() {

    /*
        センサ起動待ち
    */
    delay(500);


    //==============================================
    // WHO_AM_I確認
    //==============================================

    /*
        センサID読み取り
    */
    uint8_t whoami =
        readRegister(WHO_AM_I);

    Serial.print("WHO_AM_I = 0x");

    Serial.println(whoami, HEX);

    /*
        0xBD 以外なら異常
    */
    if (whoami != 0xBD) {

        Serial.println("LPS25HB not found");

        return false;
    }


    //==============================================
    // CTRL_REG1設定
    //==============================================

    /*
        0x90 =
        10010000b

        bit7 PD = 1
        → Power ON

        bit6-4 ODR = 001
        → 1Hz更新
    */
    writeRegister(CTRL_REG1, 0x90);

    /*
        測定開始待ち
    */
    delay(100);


    //==============================================
    // 気圧補正
    //==============================================

    /*
        +5hPa補正

        実際より高く出る場合に使う
    */
    setPressureOffset(5);

    return true;
}


//==================================================
// センサ読み取り
//==================================================

bool lps25_read(float* temperature,
                float* pressure) {

    //==============================================
    // 気圧読み取り
    //==============================================

    /*
        24bitデータを
        3byteに分けて取得
    */

    uint8_t pxl =
        readRegister(PRESS_OUT_XL);

    uint8_t pl =
        readRegister(PRESS_OUT_L);

    uint8_t ph =
        readRegister(PRESS_OUT_H);


    /*
        24bitへ合成

        ph を16bit左へ
        pl を8bit左へ
        pxl はそのまま

        最後に OR で結合
    */
    int32_t pressure_raw =
        ((int32_t)ph << 16) |
        ((int32_t)pl << 8) |
        pxl;


    /*
        データシート式

        pressure[hPa] =
        raw / 4096
    */
    float pressure_hpa =
        pressure_raw / 4096.0;


    //==============================================
    // 温度読み取り
    //==============================================

    /*
        16bit温度データ取得
    */
    uint8_t tl =
        readRegister(TEMP_OUT_L);

    uint8_t th =
        readRegister(TEMP_OUT_H);


    /*
        16bitへ結合
    */
    int16_t temp_raw =
        ((int16_t)th << 8) |
        tl;


    /*
        データシート式

        温度[℃] =
        42.5 + raw / 480
    */
    float temp_c =
        42.5 + (temp_raw / 480.0);


    //==============================================
    // 異常値除外
    //==============================================

    /*
        通信エラーなどで
        異常値になる場合があるので除外
    */
    if (pressure_hpa < 260 ||
        pressure_hpa > 1260 ||
        temp_c < -20 ||
        temp_c > 41) {

        return false;
    }


    //==============================================
    // 呼び出し元へ返す
    //==============================================

    /*
        ポインタ経由で値を返す
    */
    *temperature = temp_c;

    *pressure = pressure_hpa;

    return true;
}