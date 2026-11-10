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
#include "environment_calc.h"

#include <esp_task_wdt.h>
static uint32_t last_sensor_ms = 0;
static uint32_t boot_ms = 0;

static uint32_t last_lps25_success_ms = 0;
static uint32_t last_scd40_success_ms = 0;
static uint32_t last_sht41_success_ms = 0;

static bool last_lps25_ok = false;
static bool last_scd40_ok = false;
static bool last_sht41_ok = false;

static uint32_t sensor_seq = 0;
static uint32_t last_alive_ms = 0;


void check_app_watchdog();
void setup() {

    Serial.begin(115200);
    delay(1000);

    Serial.println("START");

    // I2C初期化
    i2c_init();

    // センサ初期化
bool lps25_ready = false;

for (int i = 0; i < 3; i++)
{
    if (lps25_init())
    {
        lps25_ready = true;
        break;
    }

    Serial.println("LPS25 init retry...");
    delay(500);
}

if (!lps25_ready)
{
    Serial.println("LPS25 init failed. Continue and let App Watchdog handle it.");
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

    boot_ms = millis();
}

void loop() {
    static float temperature = 0;
    static float pressure = 0;

    static uint16_t co2 = 0;
    static float scd_temp= 0;
    static float humidity= 0;

    static float sht_temp = 0;
    static float sht_hum = 0;

    static float absolute_humidity = 0;
    static float dew_point = 0;
    esp_task_wdt_reset();


    mqtt_update();
    
    ir_receiver_update();
    // ir_sender_update();　serialに統合
    serial_input_update();

    if (millis() - last_sensor_ms > 5000)
    {
        last_sensor_ms = millis();

        uint32_t now = millis();

        last_lps25_ok =
            lps25_read(
                &temperature,
                &pressure
            );

        if (last_lps25_ok)
        {
            last_lps25_success_ms = now;
        }

        last_scd40_ok =
            scd40_read(
                &co2,
                &scd_temp,
                &humidity
            );

        if (last_scd40_ok)
        {
            last_scd40_success_ms = now;
        }

        last_sht41_ok =
            sht41_read(
                &sht_temp,
                &sht_hum
            );

        if (last_sht41_ok)
        {
            last_sht41_success_ms = now;

            absolute_humidity =
                calc_absolute_humidity(
                    sht_temp,
                    sht_hum
                );

            dew_point =
                calc_dew_point(
                    sht_temp,
                    sht_hum
                );
        }

        if (
            last_lps25_ok
            &&
            last_scd40_ok
            &&
            last_sht41_ok
        )
        {
            sensor_seq++;
        }
    }

    // ambient_send(pressure,
    //         co2,
    //         scd_temp,
    //         humidity,
    //         sht_temp,
    //         sht_hum,
    //         absolute_humidity,
    //         dew_point
    //         );



    web_server_update(
        pressure,
        co2,
        sht_temp,
        sht_hum,
        absolute_humidity,
        dew_point
    );
    mqtt_publish_sensor_periodic(
        co2,
        sht_temp,
        sht_hum,
        pressure,
        absolute_humidity,
        dew_point
    );

    check_app_watchdog();
    
    delay(10);
}

void check_app_watchdog()
{
    const uint32_t SENSOR_TIMEOUT_MS =
        60UL * 1000UL;  // 60秒

    uint32_t now =
        millis();

    if (
        now - boot_ms > SENSOR_TIMEOUT_MS
        &&
        (
            last_lps25_success_ms == 0
            ||
            last_scd40_success_ms == 0
            ||
            last_sht41_success_ms == 0
        )
    )
    {
        Serial.println("APP WATCHDOG: sensor never started. Restarting...");
        delay(1000);
        ESP.restart();
    }

    if (
        last_lps25_success_ms > 0
        &&
        now - last_lps25_success_ms > SENSOR_TIMEOUT_MS
    )
    {
        Serial.println("APP WATCHDOG: LPS25 stale. Restarting...");
        delay(1000);
        ESP.restart();
    }

    if (
        last_scd40_success_ms > 0
        &&
        now - last_scd40_success_ms > SENSOR_TIMEOUT_MS
    )
    {
        Serial.println("APP WATCHDOG: SCD40 stale. Restarting...");
        delay(1000);
        ESP.restart();
    }

    if (
        last_sht41_success_ms > 0
        &&
        now - last_sht41_success_ms > SENSOR_TIMEOUT_MS
    )
    {
        Serial.println("APP WATCHDOG: SHT41 stale. Restarting...");
        delay(1000);
        ESP.restart();
    }
}