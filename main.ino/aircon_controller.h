#ifndef AIRCON_CONTROLLER_H
#define AIRCON_CONTROLLER_H

#include <Arduino.h>

//==========================================
// Mode
//==========================================

enum AirconMode
{
    AIRCON_MODE_OFF,
    AIRCON_MODE_AUTO,
    AIRCON_MODE_COOL,
    AIRCON_MODE_HEAT,
    AIRCON_MODE_DRY,
    AIRCON_MODE_FAN
};

//==========================================
// Fan
//==========================================

enum AirconFan
{
    AIRCON_FAN_AUTO,
    AIRCON_FAN_LOW,
    AIRCON_FAN_MED,
    AIRCON_FAN_HIGH,
    AIRCON_FAN_MAX
};

//==========================================
// Vertical Swing
//==========================================

enum AirconSwingV
{
    AIRCON_SWING_V_AUTO,
    AIRCON_SWING_V_TOP,
    AIRCON_SWING_V_UP,
    AIRCON_SWING_V_MID,
    AIRCON_SWING_V_DOWN,
    AIRCON_SWING_V_BOTTOM,
    AIRCON_SWING_V_STOP
};

//==========================================
// Horizontal Swing
//==========================================

enum AirconSwingH
{
    AIRCON_SWING_H_AUTO,
    AIRCON_SWING_H_LEFT,
    AIRCON_SWING_H_LEFT_CENTER,
    AIRCON_SWING_H_CENTER,
    AIRCON_SWING_H_RIGHT_CENTER,
    AIRCON_SWING_H_RIGHT,
    AIRCON_SWING_H_STOP
};

//==========================================
// WARP
//==========================================

enum AirconWarp
{
    AIRCON_WARP_OFF,
    AIRCON_WARP_ON
};

//==========================================
// BIO
//==========================================

enum AirconBio
{
    AIRCON_BIO_OFF,
    AIRCON_BIO_ON
};

//==========================================
// State
//==========================================

struct AirconState
{
    bool power;

    AirconMode mode;

    int temperature;

    AirconFan fan;

    AirconSwingV swing_v;

    AirconSwingH swing_h;

    AirconWarp warp;

    AirconBio bio;

    uint32_t updated_at;
};

extern AirconState aircon_state;

//==========================================
// Apply
//==========================================

void aircon_apply();

//==========================================
// Setter
//==========================================

void aircon_set_power(bool power);

void aircon_set_mode(AirconMode mode);

void aircon_set_temperature(int temp);

void aircon_set_fan(AirconFan fan);

void aircon_set_swing_v(AirconSwingV swing);

void aircon_set_swing_h(AirconSwingH swing);

void aircon_set_warp(AirconWarp warp);

void aircon_set_bio(AirconBio bio);

void aircon_set_state(
    bool power,
    AirconMode mode,
    int temperature,
    AirconFan fan,
    AirconSwingV swing_v,
    AirconSwingH swing_h,
    AirconWarp warp,
    AirconBio bio
);
//==========================================
// Enum → Text
//==========================================

const char* aircon_get_mode_text();

const char* aircon_get_fan_text();

const char* aircon_get_swing_v_text();

const char* aircon_get_swing_h_text();

const char* aircon_get_warp_text();

const char* aircon_get_bio_text();

//==========================================
// Text → Enum
//==========================================

AirconMode aircon_mode_from_text(const String& text);

AirconFan aircon_fan_from_text(const String& text);

AirconSwingV aircon_swing_v_from_text(const String& text);

AirconSwingH aircon_swing_h_from_text(const String& text);

AirconWarp aircon_warp_from_text(const String& text);

AirconBio aircon_bio_from_text(const String& text);

#endif