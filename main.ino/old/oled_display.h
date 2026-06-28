#pragma once

#include <stdint.h>

bool oled_init();

void oled_update(float pressure,
                 uint16_t co2,
                 float temperature,
                 float humidity);