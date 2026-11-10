#pragma once

bool mqtt_init();

void mqtt_update();

void mqtt_publish_light_state();

void mqtt_publish_aircon_state();

void mqtt_publish_sensor_state(
    uint16_t co2,
    float temp,
    float humidity,
    float pressure,
    float absolute_humidity,
    float dew_point
);

void mqtt_publish_sensor_periodic(
    uint16_t co2,
    float temp,
    float humidity,
    float pressure,
    float absolute_humidity,
    float dew_point
);

