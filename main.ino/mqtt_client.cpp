#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "mqtt_client.h"

#include "light_controller.h"
#include "aircon_controller.h"
#include "secrets.h"
#include "sensor_history.h"

const char* MQTT_HOST =
    "i13aaaec.ala.asia-southeast1.emqxsl.com";

const int MQTT_PORT = 8883;



const char* EMQX_CA_CERT = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";

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


    // History request; all messages are sent in small chunks later in mqtt_update().
    if (String(topic) == "room/history/request") {
        StaticJsonDocument<256> request;
        if (deserializeJson(request, payload, length)) {
            Serial.println("HISTORY: invalid JSON request");
            return;
        }
        const char *id = request["id"] | "";
        const char *client_id = request["client"] | "";
        int hours = request["hours"] | 24;
        if (!history_start_request(id, client_id, hours)) {
            Serial.println("HISTORY: rejected request (check LittleFS/time/parameters)");
        }
        return;
    }

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

    secureClient.setCACert(EMQX_CA_CERT);

    // History packets are larger than PubSubClient's default buffer.
    // Increase only once at initialization (about 1.5 KiB RAM).
    if (!mqtt.setBufferSize(1536)) {
        Serial.println("MQTT: cannot allocate larger buffer");
    }

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
            if (!mqtt.subscribe("room/history/request")) {
                Serial.println("HISTORY: subscribe request FAILED");
            }
            Serial.println("MQTT Subscribe OK");
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

    // At most one history packet per loop and every 50ms.
    // Keep a failed packet for retry instead of advancing the file cursor.
    static String queued_topic;
    static String queued_payload;
    static uint32_t last_send_ms = 0;
    if (millis() - last_send_ms >= 50 && mqtt.connected()) {
        last_send_ms = millis();
        if (queued_payload.length() == 0) {
            history_next_chunk(queued_topic, queued_payload);
        }
        if (queued_payload.length() > 0) {
            if (mqtt.publish(queued_topic.c_str(), queued_payload.c_str(), false)) {
                queued_topic = "";
                queued_payload = "";
            } else {
                Serial.println("HISTORY: publish failed; retrying");
            }
        }
    }
}

