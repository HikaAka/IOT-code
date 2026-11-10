#include <Arduino.h>

#include "aircon_controller.h"
#include "ir_sender.h"
#include "mqtt_client.h"


AirconState aircon_state =
{
    false,
    AIRCON_MODE_OFF,
    26,
    AIRCON_FAN_AUTO,
    AIRCON_SWING_V_AUTO,
    AIRCON_SWING_H_AUTO,
    AIRCON_WARP_OFF,
    AIRCON_BIO_OFF,
    0
};


void aircon_apply()
{
    if(!aircon_state.power)
    {
        sendOff();

        aircon_state.updated_at =
            millis();

        mqtt_publish_aircon_state();

        return;
    }

    switch(aircon_state.mode)
    {
        case AIRCON_MODE_COOL:

            sendCool(
                aircon_state.temperature,
                aircon_state.fan,
                aircon_state.swing_v,
                aircon_state.swing_h,
                aircon_state.warp,
                aircon_state.bio
            );

            break;

        case AIRCON_MODE_HEAT:

            sendHeat(
                aircon_state.temperature,
                aircon_state.fan,
                aircon_state.swing_v,
                aircon_state.swing_h,
                aircon_state.warp,
                aircon_state.bio
            );

            break;

        case AIRCON_MODE_DRY:

            sendDry(
                aircon_state.temperature,
                aircon_state.fan,
                aircon_state.swing_v,
                aircon_state.swing_h,
                aircon_state.warp,
                aircon_state.bio
            );

            break;

        case AIRCON_MODE_FAN:

            sendFan(
                aircon_state.fan,
                aircon_state.swing_v,
                aircon_state.swing_h,
                aircon_state.warp,
                aircon_state.bio
            );

            break;

        case AIRCON_MODE_AUTO:

            sendAuto(
                aircon_state.temperature,
                aircon_state.fan,
                aircon_state.swing_v,
                aircon_state.swing_h,
                aircon_state.warp,
                aircon_state.bio
            );

            break;

        default:

            sendOff();

            break;
    }

    aircon_state.updated_at =
        millis();

    mqtt_publish_aircon_state();
}


void aircon_set_power(bool power)
{
    aircon_state.power =
        power;

    if(!power)
    {
        aircon_state.mode =
            AIRCON_MODE_OFF;
    }

    
}


void aircon_set_mode(AirconMode mode)
{
    aircon_state.mode =
        mode;

    aircon_state.power =
        mode != AIRCON_MODE_OFF;

    
}


void aircon_set_temperature(int temp)
{
    if(temp < 18)
    {
        temp = 18;
    }

    if(temp > 30)
    {
        temp = 30;
    }

    aircon_state.temperature =
        temp;

    
}


void aircon_set_fan(AirconFan fan)
{
    aircon_state.fan =
        fan;

    
}


void aircon_set_swing_v(AirconSwingV swing)
{
    aircon_state.swing_v =
        swing;

    
}


void aircon_set_swing_h(AirconSwingH swing)
{
    aircon_state.swing_h =
        swing;

    
}


void aircon_set_warp(
    AirconWarp warp
)
{
    aircon_state.warp =
        warp;

    
}


void aircon_set_bio(
    AirconBio bio
)
{
    aircon_state.bio =
        bio;

    
}
void aircon_set_state(
    bool power,
    AirconMode mode,
    int temperature,
    AirconFan fan,
    AirconSwingV swing_v,
    AirconSwingH swing_h,
    AirconWarp warp,
    AirconBio bio
)
{
    aircon_set_power(power);

    aircon_set_mode(mode);

    aircon_set_temperature(temperature);

    aircon_set_fan(fan);

    aircon_set_swing_v(swing_v);

    aircon_set_swing_h(swing_h);

    aircon_set_warp(warp);

    aircon_set_bio(bio);
}


const char* aircon_get_mode_text()
{
    switch(aircon_state.mode)
    {
        case AIRCON_MODE_AUTO:
            return "AUTO";

        case AIRCON_MODE_COOL:
            return "COOL";

        case AIRCON_MODE_HEAT:
            return "HEAT";

        case AIRCON_MODE_DRY:
            return "DRY";

        case AIRCON_MODE_FAN:
            return "FAN";

        default:
            return "OFF";
    }
}


const char* aircon_get_fan_text()
{
    switch(aircon_state.fan)
    {
        case AIRCON_FAN_AUTO:
            return "AUTO";

        case AIRCON_FAN_LOW:
            return "LOW";

        case AIRCON_FAN_MED:
            return "MED";

        case AIRCON_FAN_HIGH:
            return "HIGH";

        case AIRCON_FAN_MAX:
            return "MAX";
    }

    return "UNKNOWN";
}


const char* aircon_get_swing_v_text()
{
    switch(aircon_state.swing_v)
    {
        case AIRCON_SWING_V_AUTO:
            return "AUTO";

        case AIRCON_SWING_V_TOP:
            return "TOP";

        case AIRCON_SWING_V_UP:
            return "UP";

        case AIRCON_SWING_V_MID:
            return "MID";

        case AIRCON_SWING_V_DOWN:
            return "DOWN";

        case AIRCON_SWING_V_BOTTOM:
            return "BOTTOM";

        case AIRCON_SWING_V_STOP:
            return "STOP";
    }

    return "UNKNOWN";
}


const char* aircon_get_swing_h_text()
{
    switch(aircon_state.swing_h)
    {
        case AIRCON_SWING_H_AUTO:
            return "AUTO";

        case AIRCON_SWING_H_LEFT:
            return "LEFT";

        case AIRCON_SWING_H_LEFT_CENTER:
            return "LEFT_CENTER";

        case AIRCON_SWING_H_CENTER:
            return "CENTER";

        case AIRCON_SWING_H_RIGHT_CENTER:
            return "RIGHT_CENTER";

        case AIRCON_SWING_H_RIGHT:
            return "RIGHT";

        case AIRCON_SWING_H_STOP:
            return "STOP";
    }

    return "UNKNOWN";
}

const char* aircon_get_warp_text()
{
    switch(aircon_state.warp)
    {
        case AIRCON_WARP_ON:

            return "ON";

        default:

            return "OFF";
    }
}

const char* aircon_get_bio_text()
{
    switch(aircon_state.bio)
    {
        case AIRCON_BIO_ON:

            return "ON";

        default:

            return "OFF";
    }
}

AirconMode aircon_mode_from_text(const String& mode)
{
    if(mode == "AUTO") return AIRCON_MODE_AUTO;
    if(mode == "COOL") return AIRCON_MODE_COOL;
    if(mode == "HEAT") return AIRCON_MODE_HEAT;
    if(mode == "DRY")  return AIRCON_MODE_DRY;
    if(mode == "FAN")  return AIRCON_MODE_FAN;

    return AIRCON_MODE_OFF;
}


AirconFan aircon_fan_from_text(const String& fan)
{
    if(fan == "LOW")  return AIRCON_FAN_LOW;
    if(fan == "MED")  return AIRCON_FAN_MED;
    if(fan == "HIGH") return AIRCON_FAN_HIGH;
    if(fan == "MAX")  return AIRCON_FAN_MAX;

    return AIRCON_FAN_AUTO;
}


AirconSwingV aircon_swing_v_from_text(const String& swing)
{
    if(swing == "TOP")    return AIRCON_SWING_V_TOP;
    if(swing == "UP")     return AIRCON_SWING_V_UP;
    if(swing == "MID")    return AIRCON_SWING_V_MID;
    if(swing == "DOWN")   return AIRCON_SWING_V_DOWN;
    if(swing == "BOTTOM") return AIRCON_SWING_V_BOTTOM;
    if(swing == "STOP")   return AIRCON_SWING_V_STOP;

    return AIRCON_SWING_V_AUTO;
}


AirconSwingH aircon_swing_h_from_text(const String& swing)
{
    if(swing == "LEFT")         return AIRCON_SWING_H_LEFT;
    if(swing == "LEFT_CENTER")  return AIRCON_SWING_H_LEFT_CENTER;
    if(swing == "CENTER")       return AIRCON_SWING_H_CENTER;
    if(swing == "RIGHT_CENTER") return AIRCON_SWING_H_RIGHT_CENTER;
    if(swing == "RIGHT")        return AIRCON_SWING_H_RIGHT;
    if(swing == "STOP")         return AIRCON_SWING_H_STOP;

    return AIRCON_SWING_H_AUTO;
}

AirconWarp aircon_warp_from_text(
    const String& text
)
{
    if(text == "ON")
    {
        return AIRCON_WARP_ON;
    }

    return AIRCON_WARP_OFF;
}

AirconBio aircon_bio_from_text(
    const String& text
)
{
    if(text == "ON")
    {
        return AIRCON_BIO_ON;
    }

    return AIRCON_BIO_OFF;
}



