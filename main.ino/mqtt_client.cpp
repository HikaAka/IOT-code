#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "mqtt_client.h"

#include "light_controller.h"
#include "aircon_controller.h"

const char* MQTT_HOST =
    "c33aa709ad5b447381d7574b4db4b5f5.s1.eu.hivemq.cloud";

const int MQTT_PORT = 8883;

const char* MQTT_USER =
    "hikaru";

const char* MQTT_PASSWORD =
    "HiveMQ1234HiveMQ";

//==========================================
// MQTT
//==========================================

WiFiClientSecure secureClient;

PubSubClient mqtt(
    secureClient
);

void mqtt_callback(
    char* topic,
    byte* payload,
    unsigned int length
)
{
    Serial.println("MQTT CALLBACK!");

    String msg;

    for (
        unsigned int i = 0;
        i < length;
        i++
    ) {
        msg += (char)payload[i];
    }

    Serial.println(
        "========== MQTT =========="
    );

    Serial.print("TOPIC : ");
    Serial.println(topic);

    Serial.print("MSG   : ");
    Serial.println(msg);


    //----------------------------------
    // ライト制御
    //----------------------------------

    if (
        String(topic)
        ==
        "room/light/all_on"
    ) {
        light_all_on();
    }

    else if (
        String(topic)
        ==
        "room/light/all_off"
    ) {
        light_off();
    }

    else if (
        String(topic)
        ==
        "room/light/night"
    ) {
        light_night();
    }

    else if (
        String(topic)
        ==
        "room/light/dim"
    ) {
        light_brightness_dim();
    }

    else if (
        String(topic)
        ==
        "room/light/timer"
    ) {
        light_timer();
    }
        //----------------------------------
    // エアコン
    //----------------------------------

    else if (
        String(topic)
        ==
        "room/aircon/set"
    )
    {
        StaticJsonDocument<384> doc;

        DeserializationError error =
            deserializeJson(doc, msg);

        if(error)
        {
            Serial.println(
                "Aircon JSON Parse Error"
            );

            return;
        }

        aircon_set_state(
            doc["power"] | aircon_state.power,

            aircon_mode_from_text(
                doc["mode"] | aircon_get_mode_text()
            ),

            doc["temperature"] | aircon_state.temperature,

            aircon_fan_from_text(
                doc["fan"] | aircon_get_fan_text()
            ),

            aircon_swing_v_from_text(
                doc["swing_v"] | aircon_get_swing_v_text()
            ),

            aircon_swing_h_from_text(
                doc["swing_h"] | aircon_get_swing_h_text()
            ),

            aircon_warp_from_text(
                doc["warp"] | aircon_get_warp_text()
            ),

            aircon_bio_from_text(
                doc["bio"] | aircon_get_bio_text()
            )
        );

        aircon_apply();
    }
}
//==========================================
// 初期化
//==========================================

bool mqtt_init() {

    secureClient.setInsecure();

    mqtt.setServer(
        MQTT_HOST,
        MQTT_PORT
    );

    Serial.println(
        "MQTT Init OK"
    );
    mqtt.setCallback(
        mqtt_callback
    );
    return true;
}


//==========================================
// 再接続
//==========================================

void mqtt_reconnect() {

    while (!mqtt.connected()) {

        Serial.println(
            "MQTT Connecting..."
        );

        if (
            mqtt.connect(
                "esp32-room",
                MQTT_USER,
                MQTT_PASSWORD
            )
        ) {

            Serial.println(
                "MQTT Connected!"
            );
            Serial.println(
                "MQTT Connected!"
            );

            mqtt.subscribe(
                "room/light/all_on"
            );

            mqtt.subscribe(
                "room/light/all_off"
            );

            mqtt.subscribe(
                "room/light/night"
            );

            mqtt.subscribe(
                "room/light/dim"
            );

            mqtt.subscribe(
                "room/light/timer"
            );
            mqtt.subscribe(
                "room/aircon/set"
            );
            Serial.println(
                "MQTT Subscribe OK"
            );
            }
        else {

            Serial.print(
                "MQTT Failed rc="
            );

            Serial.println(
                mqtt.state()
            );

            delay(5000);
        }
    }
}


//ライト状態
void mqtt_publish_light_state()
{
    String json;

    json += "{";

    json += "\"power\":";
    json += light_state.power ?
        "true" :
        "false";

    json += ",";

    json += "\"night_mode\":";
    json += light_state.night_mode ?
        "true" :
        "false";

    json += ",";

    json += "\"brightness\":\"";
    json += light_get_brightness_text();
    json += "\"";

    json += ",";

    json += "\"timer\":\"";
    json += light_get_timer_text();
    json += "\"";

    json += "}";

    mqtt.publish(
        "room/light/state",
        json.c_str(),
        true
    );

    Serial.println(json);
}
//エアコン

void mqtt_publish_aircon_state()
{
    String json;

    json += "{";

    json += "\"power\":";
    json += aircon_state.power ?
        "true" :
        "false";

    json += ",";

    json += "\"mode\":\"";
    json += aircon_get_mode_text();
    json += "\"";

    json += ",";

    json += "\"temperature\":";
    json += String(
        aircon_state.temperature
    );

    json += ",";

    json += "\"fan\":\"";
    json += aircon_get_fan_text();
    json += "\"";

    json += ",";

    json += "\"swing_v\":\"";
    json += aircon_get_swing_v_text();
    json += "\"";

    json += ",";

    json += "\"swing_h\":\"";
    json += aircon_get_swing_h_text();
    json += "\"";

    json += ",";

    json += "\"warp\":\"";
    json += aircon_get_warp_text();
    json += "\"";

    json += ",";

    json += "\"bio\":\"";
    json += aircon_get_bio_text();
    json += "\"";

    json += "}";

    mqtt.publish(
        "room/aircon/state",
        json.c_str(),
        true
    );

    Serial.println(json);
}

//センサー

void mqtt_publish_sensor_state(
    uint16_t co2,
    float temp,
    float humidity,
    float pressure,
    float absolute_humidity,
    float dew_point
)
{
    String json;

    json += "{";

    json += "\"co2\":";
    json += String(co2);

    json += ",";

    json += "\"temperature\":";
    json += String(temp,1);

    json += ",";

    json += "\"humidity\":";
    json += String(humidity,1);

    json += ",";

    json += "\"pressure\":";
    json += String(pressure,1);

    json += ",";

    json += "\"absolute_humidity\":";
    json += String(absolute_humidity, 1);

    json += ",";

    json += "\"dew_point\":";
    json += String(dew_point, 1);

    json += "}";

    mqtt.publish(
        "room/sensor/state",
        json.c_str(),
        true
    );
}
void mqtt_publish_sensor_periodic(
    uint16_t co2,
    float temp,
    float humidity,
    float pressure,
    float absolute_humidity,
    float dew_point
)
{
    static uint32_t last_publish = 0;

    if (
        millis() - last_publish
        < 60000
    ) {
        return;
    }

    mqtt_publish_sensor_state(
        co2,
        temp,
        humidity,
        pressure,
        absolute_humidity,
        dew_point
    );

    last_publish =
        millis();
}
//==========================================
// 更新
//==========================================

void mqtt_update() {

    if (!mqtt.connected()) {

        mqtt_reconnect();
    }

    mqtt.loop();
}

