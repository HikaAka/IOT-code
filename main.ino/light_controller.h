#pragma once


//==========================================
// 明るさ
//==========================================

enum LightBrightness {

    LIGHT_BRIGHTNESS_MAX,

    LIGHT_BRIGHTNESS_MID,

    LIGHT_BRIGHTNESS_LOW
};


//==========================================
// 状態
//==========================================

enum LightTimer {

    LIGHT_TIMER_OFF,

    LIGHT_TIMER_30MIN,

    LIGHT_TIMER_60MIN,

    LIGHT_TIMER_SLEEP
};
struct LightState {

    bool power;

    bool night_mode;

    LightBrightness brightness;

    LightTimer timer;
};

//==========================================
// グローバル状態
//==========================================

extern LightState light_state;


//==========================================
// API
//==========================================

void light_init();

void light_all_on();

void light_off();

void light_night();

void light_brightness_dim();

void light_timer();

const char* light_get_brightness_text();

const char* light_get_timer_text();