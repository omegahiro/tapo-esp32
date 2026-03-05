#pragma once

#include "tapo_protocol.h"
#include <time.h>

class TapoDevice {
public:
    static constexpr int kSendRetries = 3;
    static constexpr int kReconnectRetries = 3;
    static constexpr const char* kTerminalUUID = "00-00-00-00-00-00";

    bool begin(const String& ip, const String& user, const String& pass) {
        ready_ = protocol_.handshake(ip, user, pass);
        if (!ready_) Serial.println("TAPO_DEVICE: handshake failed");
        return ready_;
    }

    bool on()  { return set_on(true); }
    bool off() { return set_on(false); }

    bool set_brightness(uint8_t level) { // 1-100
        return send_set_device_info("{\"brightness\":" + String(level) + "}", "\"brightness\":" + String(level));
    }

    bool set_color(uint16_t hue, uint8_t sat) { // hue 0-360, sat 0-100
        String params = "{\"hue\":" + String(hue) + ",\"saturation\":" + String(sat) + ",\"color_temp\":0}";
        return send_set_device_info(params, "\"hue\":" + String(hue));
    }

    bool set_color_temperature(uint16_t kelvin) { // e.g. 2500-6500
        String params = "{\"color_temp\":" + String(kelvin) + ",\"hue\":0,\"saturation\":0}";
        return send_set_device_info(params, "\"color_temp\":" + String(kelvin));
    }

    bool set_hue_saturation(uint16_t hue, uint8_t sat) { return set_color(hue, sat); }

private:
    TapoProtocol protocol_;
    bool ready_ = false;

    static uint64_t request_time_millis() {
        time_t sec = time(nullptr);
        if (sec <= 100000) return (uint64_t)millis(); // Fallback when NTP time is not ready
        return (uint64_t)sec * 1000ULL + (uint64_t)(millis() % 1000);
    }

    static String add_meta(String json) {
        json.trim();
        int end = json.lastIndexOf('}');
        if (end < 0) return json;
        json.remove(end);
        json += ",\"requestTimeMilis\":" + String((uint64_t)request_time_millis());
        json += ",\"terminalUUID\":\"" + String(kTerminalUUID) + "\"}";
        return json;
    }

    String get_device_info() {
        return protocol_.send_message(add_meta("{\"method\":\"get_device_info\",\"params\":{}}"));
    }

    bool send_raw(const String& json, const String& expected = "") {
        if (!ready_) {
            Serial.println("TAPO_DEVICE: not initialized");
            return false;
        }

        const String msg = add_meta(json);

        for (int r = 0; r < kReconnectRetries; ++r) {
            for (int s = 0; s < kSendRetries; ++s) {
                String resp = protocol_.send_message(msg);
                if (!resp.length()) continue;
                if (expected.length() == 0) return true;
                if (get_device_info().indexOf(expected) != -1) return true;
            }
            protocol_.rehandshake();
        }
        return false;
    }

    bool set_on(bool on) {
        String params = String("{\"device_on\":") + (on ? "true" : "false") + "}";
        String expected = String("\"device_on\":") + (on ? "true" : "false");
        return send_set_device_info(params, expected);
    }

    bool send_set_device_info(const String& params_obj, const String& expected) {
        String cmd = "{\"method\":\"set_device_info\",\"params\":" + params_obj + "}";
        return send_raw(cmd, expected);
    }
};