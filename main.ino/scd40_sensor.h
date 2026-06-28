#pragma once

#include <stdint.h>

bool scd40_init();

bool scd40_read(uint16_t* co2,
                float* temperature,
                float* humidity);

// void scd40_set_pressure(float pressure_hpa);