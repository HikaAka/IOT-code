#include <Arduino.h>

#include "serial_input.h"

#include "aircon_controller.h"

void serial_input_update() {

    if (!Serial.available()) {

        return;
    }

    //----------------------------------------
    // コマンド取得
    //----------------------------------------

    String cmd =
        Serial.readStringUntil('\n');

    cmd.trim();

    Serial.print("Serial Command : ");

    Serial.println(cmd);

    //----------------------------------------
    // OFF
    //----------------------------------------

    if (cmd == "off") {

        aircon_set_power(false);

        aircon_apply();

        return;
    }

    if (cmd.startsWith("c")) {

        int temp =
            cmd.substring(1).toInt();

        aircon_set_mode(
            AIRCON_MODE_COOL
        );

        aircon_set_temperature(
            temp
        );

        aircon_apply();

        return;
    }

    if (cmd.startsWith("h")) {

        int temp =
            cmd.substring(1).toInt();

        aircon_set_mode(
            AIRCON_MODE_HEAT
        );

        aircon_set_temperature(
            temp
        );

        aircon_apply();

        return;
    }
}