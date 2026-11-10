#pragma once

#include <stdint.h>

bool ambient_init();

bool ambient_send(float pressure,
                  uint16_t co2,
                  float temperature,
                  float humidity,
                  float sht_temperature,
                  float sht_humidity,
                  float absolute_humidity,
                  float dew_point
                  
                  );