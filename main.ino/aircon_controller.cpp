#include <Arduino.h>

#include "aircon_controller.h"

#include "ir_sender.h"

#include "mqtt_client.h"

AirconState aircon_state = {

    false,

    AIRCON_MODE_OFF,

    22,

    0
};
void aircon_set_cool(int temp) {

    sendCool(temp);

    aircon_state.power = true;

    aircon_state.mode =
        AIRCON_MODE_COOL;

    aircon_state.temperature =
        temp;

    aircon_state.updated_at =
        millis();

    mqtt_publish_aircon_state();
}

void aircon_set_heat(int temp) {

    sendHeat(temp);

    aircon_state.power = true;

    aircon_state.mode =
        AIRCON_MODE_HEAT;

    aircon_state.temperature =
        temp;

    aircon_state.updated_at =
        millis();

    mqtt_publish_aircon_state();
}
void aircon_off() {

    sendOff();

    aircon_state.power = false;

    aircon_state.mode =
        AIRCON_MODE_OFF;

    aircon_state.updated_at =
        millis();

    mqtt_publish_aircon_state();
}

const char* aircon_get_mode_text()
{
    switch(
        aircon_state.mode
    )
    {
        case AIRCON_MODE_COOL:
            return "COOL";

        case AIRCON_MODE_HEAT:
            return "HEAT";

        default:
            return "OFF";
    }
}