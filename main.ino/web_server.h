#ifndef WEB_SERVER_H
#define WEB_SERVER_H

bool web_server_init();

void web_server_update(float pressure,
                  uint16_t co2,
                  float sht_temperature,
                  float sht_humidity                  
                  );

void handleAircon();

#endif