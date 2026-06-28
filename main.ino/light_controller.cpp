#include <Arduino.h>

#include "light_controller.h"

#include "ir_sender.h"

#include "mqtt_client.h"
//==========================================
// 状態
//==========================================

LightState light_state;


//==========================================
// 初期化
//==========================================

void light_init() {

    light_state.power = false;

    light_state.night_mode = false;

    light_state.brightness =
        LIGHT_BRIGHTNESS_MAX;

    light_state.timer =
    LIGHT_TIMER_OFF;
}


//==========================================
// 全灯
//==========================================

void light_all_on() {

    ir_send_light_all_on();

    light_state.power = true;

    light_state.night_mode = false;

    light_state.brightness =
        LIGHT_BRIGHTNESS_MAX;

    mqtt_publish_light_state();
}


//==========================================
// OFF
//==========================================

void light_off() {

    ir_send_light_off();

    light_state.power = false;

    light_state.night_mode = false;

    mqtt_publish_light_state();
}


//==========================================
// 常夜灯
//==========================================

void light_night() {

    ir_send_light_night();

    light_state.power = true;

    light_state.night_mode = true;

    mqtt_publish_light_state();

}


//==========================================
// 調光
//==========================================

void light_brightness_dim() {

    ir_send_light_brightness_dim();

    light_state.power = true;

    light_state.night_mode = false;


    //--------------------------------------
    // 状態循環
    //--------------------------------------

    switch (
        light_state.brightness
    ) {

        case LIGHT_BRIGHTNESS_MAX:

            light_state.brightness =
                LIGHT_BRIGHTNESS_MID;

            break;


        case LIGHT_BRIGHTNESS_MID:

            light_state.brightness =
                LIGHT_BRIGHTNESS_LOW;

            break;


        case LIGHT_BRIGHTNESS_LOW:

            light_state.brightness =
                LIGHT_BRIGHTNESS_MAX;

            break;
    }

    mqtt_publish_light_state();
}


//==========================================
// タイマー
//==========================================

void light_timer() {

    ir_send_light_timer();


    switch(light_state.timer) {

        case LIGHT_TIMER_OFF:

            light_state.timer =
                LIGHT_TIMER_30MIN;

            break;


        case LIGHT_TIMER_30MIN:

            light_state.timer =
                LIGHT_TIMER_60MIN;

            break;


        case LIGHT_TIMER_60MIN:

            light_state.timer =
                LIGHT_TIMER_SLEEP;

            break;


        default:

            light_state.timer =
                LIGHT_TIMER_OFF;

            break;
    }

    mqtt_publish_light_state();
    Serial.println(
        "Light timer changed"
    );
}
const char* light_get_brightness_text() {

    switch(light_state.brightness) {

        case LIGHT_BRIGHTNESS_MAX:
            return "MAX";

        case LIGHT_BRIGHTNESS_MID:
            return "MID";

        case LIGHT_BRIGHTNESS_LOW:
            return "LOW";
    }

    return "UNKNOWN";
}

const char* light_get_timer_text() {

    switch(light_state.timer) {

        case LIGHT_TIMER_OFF:

            return "OFF";


        case LIGHT_TIMER_30MIN:

            return "30MIN";


        case LIGHT_TIMER_60MIN:

            return "60MIN";


        case LIGHT_TIMER_SLEEP:

            return "SLEEP";
    }

    return "UNKNOWN";
}