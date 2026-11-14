#include "sensor_history.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <math.h>
#include <time.h>

// FIRST BOOT ONLY: if the filesystem is unformatted and contains NO data to
// preserve, change to true once, upload, then return to false and upload again.
// WARNING: Formatting removes files from the selected filesystem partition.
static constexpr bool FORMAT_IF_MOUNT_FAILS = false;
static constexpr uint32_t MIN_VALID_UNIX = 1700000000UL;
static constexpr uint32_t SLOT_SECONDS = 600UL;
static constexpr uint32_t FILE_SECONDS = 7UL * 86400UL; // 1 file per UTC week
static constexpr uint32_t RETAIN_WEEKS = 53; // about 1 year
static constexpr int POINTS_PER_PACKET = 16;

struct __attribute__((packed)) SensorRecord {
    uint32_t unix_time;
    int16_t temperature_x10;
    uint16_t humidity_x10;
    uint16_t co2;
    uint16_t pressure_x10;
    uint16_t absolute_humidity_x10;
    int16_t dew_point_x10;
};
static_assert(sizeof(SensorRecord) == 16, "SensorRecord must be 16 bytes");

static bool mounted = false;
static uint32_t last_saved_slot = UINT32_MAX;
static uint32_t last_cleanup_week = UINT32_MAX;

struct HistoryReader {
    bool active = false;
    uint32_t from = 0;
    uint32_t through = 0;
    uint32_t week = 0;
    uint32_t last_week = 0;
    uint32_t sequence = 0;
    uint32_t sent_records = 0;
    String id;
    String topic;
    File file;
};
static HistoryReader reader;

static String weekly_path(uint32_t week) {
    return String("/h") + String(week) + ".bin";
}

static void cleanup_old_weeks(uint32_t current_week) {
    if (!mounted || last_cleanup_week == current_week) return;
    last_cleanup_week = current_week;
    File dir = LittleFS.open("/");
    if (!dir || !dir.isDirectory()) return;
    File f = dir.openNextFile();
    while (f) {
        String name = String(f.name());
        f.close();
        if (name.startsWith("/")) name.remove(0, 1);
        if (name.startsWith("h") && name.endsWith(".bin")) {
            const String digits = name.substring(1, name.length() - 4);
            bool numeric = digits.length() > 0;
            for (unsigned int i = 0; i < digits.length(); ++i) {
                if (digits[i] < '0' || digits[i] > '9') numeric = false;
            }
            if (numeric) {
                uint32_t old_week = uint32_t(digits.toInt());
                if (old_week < current_week && current_week - old_week > RETAIN_WEEKS) {
                    String path = String("/") + name;
                    LittleFS.remove(path.c_str());
                }
            }
        }
        f = dir.openNextFile();
    }
    dir.close();
}

bool history_init() {
    mounted = LittleFS.begin(FORMAT_IF_MOUNT_FAILS);
    if (!mounted) {
        Serial.println("HISTORY: LittleFS mount FAILED (format disabled)");
        return false;
    }
    Serial.printf("HISTORY: LittleFS total=%u used=%u bytes\n",
                  (unsigned)LittleFS.totalBytes(), (unsigned)LittleFS.usedBytes());
    return true;
}

void history_record_if_due(uint16_t co2, float temperature, float humidity,
                           float pressure, float absolute_humidity, float dew_point) {
    if (!mounted) return;
    time_t raw_now = time(nullptr);
    if (raw_now < time_t(MIN_VALID_UNIX)) return;
    uint32_t now = uint32_t(raw_now);
    const uint32_t slot = now / SLOT_SECONDS;
    if (slot == last_saved_slot) return;
    if (!isfinite(temperature) || !isfinite(humidity) || !isfinite(pressure) ||
        !isfinite(absolute_humidity) || !isfinite(dew_point) ||
        temperature < -40 || temperature > 85 || humidity < 0 || humidity > 100 ||
        pressure < 300 || pressure > 1200 || absolute_humidity < 0 || absolute_humidity > 100 ||
        dew_point < -80 || dew_point > 85 || co2 == 0) return;

    SensorRecord r = {};
    r.unix_time = slot * SLOT_SECONDS;
    r.temperature_x10 = int16_t(lroundf(temperature * 10));
    r.humidity_x10 = uint16_t(lroundf(humidity * 10));
    r.co2 = co2;
    r.pressure_x10 = uint16_t(lroundf(pressure * 10));
    r.absolute_humidity_x10 = uint16_t(lroundf(absolute_humidity * 10));
    r.dew_point_x10 = int16_t(lroundf(dew_point * 10));

    const uint32_t current_week = r.unix_time / FILE_SECONDS;
    cleanup_old_weeks(current_week);
    const String path = weekly_path(current_week);
    // Avoid duplicate timestamp on a restart in the same 10-minute slot.
    File previous = LittleFS.open(path.c_str(), "r");
    if (previous) {
        if (previous.size() >= sizeof(SensorRecord) &&
            previous.seek(previous.size() - sizeof(SensorRecord), SeekSet)) {
            SensorRecord last = {};
            if (previous.readBytes((char *)&last, sizeof(last)) == sizeof(last) &&
                last.unix_time >= r.unix_time) {
                last_saved_slot = slot;
                previous.close();
                return;
            }
        }
        previous.close();
    }
    File file = LittleFS.open(path.c_str(), "a");
    if (!file) { Serial.println("HISTORY: file open FAILED"); return; }
    size_t written = file.write((const uint8_t *)&r, sizeof(r));
    file.close();
    if (written == sizeof(r)) {
        last_saved_slot = slot;
        Serial.printf("HISTORY: saved %lu, used=%u bytes\n",
                      (unsigned long)r.unix_time, (unsigned)LittleFS.usedBytes());
    } else {
        Serial.println("HISTORY: write FAILED (out of space?)");
    }
}

static bool is_safe_token(const char *value, int max_length) {
    if (!value) return false;
    int n = 0;
    for (const char *p = value; *p; ++p) {
        if (++n > max_length) return false;
        const char c = *p;
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_')) return false;
    }
    return n > 0;
}

bool history_start_request(const char *request_id, const char *client_id, int hours) {
    if (!mounted || !is_safe_token(request_id, 32) ||
        !is_safe_token(client_id, 48) || hours < 1 || hours > 720) return false;
    if (reader.file) reader.file.close();
    time_t raw_now = time(nullptr);
    if (raw_now < time_t(MIN_VALID_UNIX)) return false;
    uint32_t now = uint32_t(raw_now);
    reader.active = true;
    reader.from = now - uint32_t(hours) * 3600UL;
    reader.through = now;
    reader.week = reader.from / FILE_SECONDS;
    reader.last_week = reader.through / FILE_SECONDS;
    reader.sequence = 0;
    reader.sent_records = 0;
    reader.id = String(request_id);
    reader.topic = String("room/history/response/") + client_id;
    Serial.printf("HISTORY: request %s, last %d hours\n", request_id, hours);
    return true;
}

bool history_next_chunk(String &out_topic, String &out_payload) {
    if (!reader.active) return false;
    out_topic = reader.topic;
    out_payload = String("{\"id\":\"") + reader.id + "\",\"seq\":" + String(reader.sequence) + ",\"rows\":[";
    int count = 0;
    while (count < POINTS_PER_PACKET && reader.week <= reader.last_week) {
        if (!reader.file) {
            const String path = weekly_path(reader.week);
            reader.file = LittleFS.open(path.c_str(), "r");
            if (!reader.file) { ++reader.week; continue; }
        }
        SensorRecord r = {};
        if (reader.file.readBytes((char *)&r, sizeof(r)) != sizeof(r)) {
            reader.file.close();
            ++reader.week;
            continue;
        }
        if (r.unix_time < reader.from || r.unix_time > reader.through) continue;
        if (count) out_payload += ',';
        out_payload += '[';
        out_payload += String(r.unix_time); out_payload += ',';
        out_payload += String(r.temperature_x10); out_payload += ',';
        out_payload += String(r.humidity_x10); out_payload += ',';
        out_payload += String(r.co2); out_payload += ',';
        out_payload += String(r.pressure_x10); out_payload += ',';
        out_payload += String(r.absolute_humidity_x10); out_payload += ',';
        out_payload += String(r.dew_point_x10); out_payload += ']';
        ++count;
    }
    if (count > 0) {
        reader.sent_records += count;
        ++reader.sequence;
        out_payload += "]}";
        return true;
    }
    if (reader.file) reader.file.close();
    out_payload = String("{\"id\":\"") + reader.id + "\",\"seq\":" +
                  String(reader.sequence) + ",\"done\":true,\"total\":" +
                  String(reader.sent_records) + "}";
    reader.active = false;
    return true;
}
