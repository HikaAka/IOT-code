#pragma once

bool lps25_init();

bool lps25_read(float* temperature,
                float* pressure);