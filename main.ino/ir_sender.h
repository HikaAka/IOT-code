#pragma once

bool ir_sender_init();

// void ir_sender_update();

//エアコン

void sendCool(int temp);

void sendHeat(int temp);

void sendOff();

//ライト
void ir_send_light_all_on();

void ir_send_light_off();

void ir_send_light_night();

void ir_send_light_brightness_dim();

void ir_send_light_timer();