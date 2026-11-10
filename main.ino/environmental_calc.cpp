#include <Arduino.h>
#include <math.h>

#include "environment_calc.h"

float calc_absolute_humidity(
    float temp,
    float humidity
)
{
    float e =
        6.1078 *
        pow(
            10,
            (7.5 * temp) /
            (237.3 + temp)
        );

    e *= humidity / 100.0;

    return
        216.7 *
        e /
        (273.15 + temp);
}

float calc_dew_point(
    float temp,
    float humidity
)
{
    float a = 17.27;
    float b = 237.7;

    float alpha =
        ((a * temp) / (b + temp))
        +
        log(humidity / 100.0);

    return
        (b * alpha) /
        (a - alpha);
}