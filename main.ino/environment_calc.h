#ifndef ENVIRONMENT_CALC_H
#define ENVIRONMENT_CALC_H

float calc_absolute_humidity(
    float temp,
    float humidity
);

float calc_dew_point(
    float temp,
    float humidity
);

#endif