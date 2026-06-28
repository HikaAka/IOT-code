#pragma once

bool sht41_init();

bool sht41_read(float* temp,
                float* hum);