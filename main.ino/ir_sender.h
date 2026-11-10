#ifndef IR_SENDER_H
#define IR_SENDER_H

#include <Arduino.h>

#include "aircon_controller.h"

bool ir_sender_init();

void sendCool(
    int temp,
    AirconFan fan,
    AirconSwingV swing_v,
    AirconSwingH swing_h,
    AirconWarp warp,
    AirconBio bio
);

void sendHeat(
    int temp,
    AirconFan fan,
    AirconSwingV swing_v,
    AirconSwingH swing_h,
    AirconWarp warp,
    AirconBio bio
);

void sendDry(
    int temp,
    AirconFan fan,
    AirconSwingV swing_v,
    AirconSwingH swing_h,
    AirconWarp warp,
    AirconBio bio
);

void sendFan(
    AirconFan fan,
    AirconSwingV swing_v,
    AirconSwingH swing_h,
    AirconWarp warp,
    AirconBio bio
);

void sendAuto(
    int temp,
    AirconFan fan,
    AirconSwingV swing_v,
    AirconSwingH swing_h,
    AirconWarp warp,
    AirconBio bio
);

void sendOff();

#endif
//ライト
void ir_send_light_all_on();

void ir_send_light_off();

void ir_send_light_night();

void ir_send_light_brightness_dim();

void ir_send_light_timer();