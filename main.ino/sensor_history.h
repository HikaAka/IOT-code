#pragma once
#include <Arduino.h>
#include <stdint.h>

// LittleFS mount. false = filesystem could not be mounted.
bool history_init();

// Call after a successful read of ALL sensors, typically every 5 s.
// Writes at most once in each 10-minute UTC time slot.
void history_record_if_due(uint16_t co2, float temperature, float humidity,
                           float pressure, float absolute_humidity, float dew_point);

// MQTT history request / streaming (reply is split into small JSON packets).
bool history_start_request(const char *request_id, const char *client_id, int hours);
bool history_next_chunk(String &out_topic, String &out_payload);
