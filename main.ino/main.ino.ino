#include "i2c_manager.h"
#include "lps25_sensor.h"
#include "scd40_sensor.h"

#include "wifi_manager.h"

#include "ambient_client.h"
#include "ir_receiver.h"
#include "ir_sender.h"
#include "sht41_sensor.h"
#include "web_server.h"
#include "aircon_controller.h"
#include "serial_input.h"
#include "light_controller.h"
#include "mqtt_client.h"

#include <esp_task_wdt.h>

void setup() {

    Serial.begin(115200);
    delay(1000);

    Serial.println("START");

    // I2C初期化
    i2c_init();

    // センサ初期化
    if (!lps25_init()) {

        Serial.println("Sensor Error");

        while (1) {
            delay(10);
        }
    }
    scd40_init();
    wifi_connect();
    ambient_init(); 
    ir_receiver_init();
    ir_sender_init();
    sht41_init();
    web_server_init();
    light_init();
    mqtt_init();

    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 30000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };

    esp_task_wdt_init(&wdt_config);

    esp_task_wdt_add(NULL);

    Serial.println(
        "Task Watchdog Started"
    );
}
static uint32_t last_sensor_ms = 0;


void loop() {

    float temperature;
    float pressure;

    uint16_t co2;
    float scd_temp;
    float humidity;

    float sht_temp;
    float sht_hum;

    if (millis() - last_sensor_ms > 5000)
{
    last_sensor_ms = millis();

    if (lps25_read(&temperature,
                   &pressure)) {
    } 
        if (scd40_read(&co2,
                &scd_temp,
                &humidity)) {
    }
        if (sht41_read(&sht_temp,
                &sht_hum)) {
    }
}

    ambient_send(pressure,
            co2,
            scd_temp,
            humidity,
            sht_temp,
            sht_hum
            );

    ir_receiver_update();
    // ir_sender_update();　serialに統合
    serial_input_update();

    web_server_update(
        pressure,
        co2,
        sht_temp,
        sht_hum
    );
    mqtt_publish_sensor_periodic(
    co2,
    sht_temp,
    sht_hum,
    pressure
    );
    mqtt_update();

    esp_task_wdt_reset();

    
    delay(10);
}