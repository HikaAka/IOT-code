#include <Arduino.h>

#include <WiFi.h>

#include <WebServer.h>

#include "web_server.h"

#include "aircon_controller.h"

#include "light_controller.h"



//==================================================
// WebServer
//==================================================

WebServer server(80);


//==================================================
// 最新値保存
//==================================================

static float g_pressure;

static uint16_t g_co2;

static float g_sht_temp;

static float g_sht_humidity;

static float g_absolute_humidity;
static float g_dew_point;


//==================================================
// TOPページ
//==================================================

void handleRoot() {

    String html;


    //------------------------------------------
    // HTML開始
    //------------------------------------------

    html += "<html>";

    html += "<head>";

    html += "<meta charset='utf-8'>";

    html += "<title>ESP32 Room Monitor</title>";


    //------------------------------------------
    // CSS
    //------------------------------------------

    html += "<style>";

    html += "body{";
    html += "font-family:sans-serif;";
    html += "text-align:center;";
    html += "}";

    html += "button{";
    html += "width:120px;";
    html += "height:70px;";
    html += "font-size:24px;";
    html += "margin:10px;";
    html += "border-radius:12px;";
    html += "}";

    html += ".tempBtn{";
    html += "width:80px;";
    html += "}";

    html += ".tempValue{";
    html += "font-size:48px;";
    html += "}";

    html += "</style>";

    html += "</head>";


    //------------------------------------------
    // BODY開始
    //------------------------------------------

    html += "<body>";


    //==================================================
    // センサ表示
    //==================================================

    html += "<h1>Room Sensor</h1>";


    html += "<p>CO2 : ";
    html += String(g_co2);
    html += " ppm</p>";


    html += "<p>Pressure : ";
    html += String(g_pressure);
    html += " hPa</p>";


    html += "<h2>SHT41</h2>";


    html += "<p>Temp : ";
    html += String(g_sht_temp);
    html += " C</p>";


    html += "<p>Humidity : ";
    html += String(g_sht_humidity);
    html += " %</p>";

    html += "<p>Absolute Humidity : ";
    html += String(g_absolute_humidity);
    html += " g/m^3</p>";

    html += "<p>Dew Point : ";
    html += String(g_dew_point);
    html += " C </p>";
    //==================================================
    // エアコン状態
    //==================================================

    html += "<hr>";

    html += "<h1>Aircon</h1>";


    //------------------------------------------
    // Power
    //------------------------------------------

    html += "<p>Power : ";

    html += aircon_state.power ?
        "ON" :
        "OFF";

    html += "</p>";


    //------------------------------------------
    // Mode
    //------------------------------------------

    html += "<p>Mode : ";


    switch (aircon_state.mode) {

        case AIRCON_MODE_COOL:

            html += "COOL";

            break;

        case AIRCON_MODE_HEAT:

            html += "HEAT";

            break;

        default:

            html += "OFF";

            break;
    }

    html += "</p>";


    //------------------------------------------
    // 温度表示
    //------------------------------------------

    html += "<div class='tempValue'>";

    html += String(
        aircon_state.temperature
    );

    html += " C";

    html += "</div>";


    //------------------------------------------
    // 温度変更ボタン
    //------------------------------------------

    html += "<a href='/tempdown'>";

    html += "<button class='tempBtn'>";

    html += "-";

    html += "</button>";

    html += "</a>";


    html += "<a href='/tempup'>";

    html += "<button class='tempBtn'>";

    html += "+";

    html += "</button>";

    html += "</a>";


    html += "<br><br>";


    //------------------------------------------
    // COOL
    //------------------------------------------

    html += "<a href='/cool'>";

    html += "<button>";

    html += "COOL";

    html += "</button>";

    html += "</a>";


    //------------------------------------------
    // HEAT
    //------------------------------------------

    html += "<a href='/heat'>";

    html += "<button>";

    html += "HEAT";

    html += "</button>";

    html += "</a>";


    //------------------------------------------
    // OFF
    //------------------------------------------

    html += "<a href='/off'>";

    html += "<button>";

    html += "OFF";

    html += "</button>";

    html += "</a>";

    html += "<a href='/aircon/apply'>";
    html += "<button>APPLY</button>";
    html += "</a>";

    
    //------------------------------------------
    // ライト
    //------------------------------------------
    html += "<hr>";

    html += "<h1>Light</h1>";

    html += "<p>Power : ";

    html += light_state.power ?
        "ON" :
        "OFF";

    html += "</p>";
    html += "<p>Night Mode : ";

    html += light_state.night_mode ?
        "ON" :
        "OFF";

    html += "</p>";
    html += "<p>Brightness : ";

    html += light_get_brightness_text();

    html += "</p>";

    html += "<p>Timer : ";

    html += light_get_timer_text();

    html += "</p>";

    html += "<h2>Light Control</h2>";

    html += "<a href='/light/all_on'><button>ALL ON</button></a>";
    html += "<a href='/light/all_off'><button>ALL OFF</button></a>";

    html += "<a href='/light/night'><button>NIGHT MODE</button></a>";
    html += "<a href='/light/dim'><button>DIM</button></a>";

    html += "<a href='/light/timer'><button>TIMER 10min</button></a>";

    //------------------------------------------
    // HTML終了
    //------------------------------------------

    html += "</body>";

    html += "</html>";


    server.send(
        200,
        "text/html",
        html
    );
}


//==================================================
// COOL
//==================================================

void handleCool() {

    aircon_set_mode(
        AIRCON_MODE_COOL
    );

    aircon_set_temperature(
        aircon_state.temperature
    );

    aircon_apply();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}

//==================================================
// HEAT
//==================================================

void handleHeat() {

    aircon_set_mode(
        AIRCON_MODE_HEAT
    );

    aircon_set_temperature(
        aircon_state.temperature
    );

    aircon_apply();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}


//==================================================
// OFF
//==================================================

void handleOff() {

    aircon_set_power(
        false
    );

    aircon_apply();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}

//==================================================
// 温度 +
//==================================================

void handleTempUp() {

    aircon_set_temperature(
        aircon_state.temperature + 1
    );

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}

//==================================================
// 温度 -
//==================================================

void handleTempDown() {

    aircon_set_temperature(
        aircon_state.temperature - 1
    );

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}

void handleAirconApply() {

    aircon_apply();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}
//ライト
void handleLightAllOn() {

    light_all_on();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}
void handleLightOff() {

    light_off();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}
void handleLightNight() {

    light_night();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}
void handleLightDim() {

    light_brightness_dim();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}
void handleLightTimer() {

    light_timer();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(303);
}

//==================================================
// JSON API
//==================================================

void handleSensorApi() {

    String json;

    json += "{";

    json += "\"co2\":";
    json += String(g_co2);
    json += ",";

    json += "\"pressure\":";
    json += String(g_pressure);
    json += ",";

    json += "\"sht_temperature\":";
    json += String(g_sht_temp);
    json += ",";

    json += "\"sht_humidity\":";
    json += String(g_sht_humidity);

    json += "}";


    server.send(
        200,
        "application/json",
        json
    );
}


//==================================================
// 初期化
//==================================================

bool web_server_init() {

    //------------------------------------------
    // TOP
    //------------------------------------------

    server.on("/", handleRoot);


    //------------------------------------------
    // API
    //------------------------------------------

    server.on(
        "/api/sensor",
        handleSensorApi
    );


    //------------------------------------------
    // エアコン
    //------------------------------------------

    server.on("/cool", handleCool);

    server.on("/heat", handleHeat);

    server.on("/off", handleOff);

    server.on("/tempup", handleTempUp);

    server.on("/tempdown", handleTempDown);

    server.on(
    "/aircon/apply",
    handleAirconApply
);
    //------------------------------------------
    // ライト
    //------------------------------------------


    server.on(
        "/light/all_on",
        handleLightAllOn
    );

    server.on(
        "/light/all_off",
        handleLightOff
    );

    server.on(
        "/light/night",
        handleLightNight
    );

    server.on(
        "/light/dim",
        handleLightDim
    );

    server.on(
        "/light/timer",
        handleLightTimer
    );


    //------------------------------------------
    // サーバ開始
    //------------------------------------------

    server.begin();

    Serial.println("Web Server Started");

    return true;
}


//==================================================
// 更新
//==================================================

void web_server_update(
    float pressure,
    uint16_t co2,
    float sht_temp,
    float sht_humidity,
    float absolute_humidity,
    float dew_point 
){

    //------------------------------------------
    // 最新値保存
    //------------------------------------------

    g_pressure = pressure;

    g_co2 = co2;

    g_sht_temp = sht_temp;

    g_sht_humidity = sht_humidity;
    g_absolute_humidity = absolute_humidity;
    g_dew_point =  dew_point;
    //------------------------------------------
    // Web処理
    //------------------------------------------

    server.handleClient();
}