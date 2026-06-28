#ifndef AIRCON_CONTROLLER_H
#define AIRCON_CONTROLLER_H

const char* aircon_get_mode_text();

enum AirconMode {

    AIRCON_MODE_OFF,

    AIRCON_MODE_COOL,

    AIRCON_MODE_HEAT
};


struct AirconState {

    bool power;

    AirconMode mode;

    int temperature;

    unsigned long updated_at;
};


extern AirconState aircon_state;


void aircon_set_cool(int temp);

void aircon_set_heat(int temp);

void aircon_off();

#endif