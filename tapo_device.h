#pragma once

#include "tapo_protocol.h"

#define TAPO_MAX_SEND_RETRIES      3
#define TAPO_MAX_RECONNECT_RETRIES 3

class TapoDevice {
private:
    TapoProtocol protocol;
    bool ready = false;

    bool check_state(const String &expected_state) {
        if (!ready) return false;
        String response = protocol.send_message("{\"method\":\"get_device_info\",\"params\":{}}");
        if (response.length() == 0) return false;
        return response.indexOf(expected_state) != -1;
    }

    bool send_command(const String &command, const String &expected_state = "") {
        if (!ready) {
            Serial.println("TAPO_DEVICE: Not initialized. Call begin() and check its return value.");
            return false;
        }

        for (int i = 0; i < TAPO_MAX_RECONNECT_RETRIES; i++) {
            for (int j = 0; j < TAPO_MAX_SEND_RETRIES; j++) {
                String resp = protocol.send_message(command);
                if (resp.length() == 0) {
                    // WiFi/HTTP/decrypt fail → retry
                    continue;
                }
                if (expected_state == "" || check_state(expected_state)) {
                    return true;
                }
            }
            Serial.println("TAPO_DEVICE: Failed to send command, rehandshaking...");
            protocol.rehandshake();
        }

        Serial.println("TAPO_DEVICE: Giving up command");
        return false;
    }

    String wrap_param(const String &key, const String &value, bool numeric = false) {
        return "\"" + key + "\":" + (numeric ? value : "\"" + value + "\"");
    }

public:
    bool begin(const String &ip_address, const String &username, const String &password) {
        ready = protocol.handshake(ip_address, username, password);
        if (!ready) {
            Serial.println("TAPO_DEVICE: handshake failed (WiFi/IP/credentials/Third-Party Compatibility)");
        }
        return ready;
    }

    bool on() {
        const String command = "{\"method\":\"set_device_info\",\"params\":{\"device_on\":true}}";
        const String expected = "\"device_on\":true";
        return send_command(command, expected);
    }

    bool off() {
        const String command = "{\"method\":\"set_device_info\",\"params\":{\"device_on\":false}}";
        const String expected = "\"device_on\":false";
        return send_command(command, expected);
    }

    bool set_brightness(uint8_t level) { // 1-100
        String cmd = "{\"method\":\"set_device_info\",\"params\":{" +
                     wrap_param("brightness", String(level), true) + "}}";
        String exp = "\"brightness\":" + String(level);
        return send_command(cmd, exp);
    }

    bool set_color(uint16_t hue, uint8_t saturation) { // hue 0-360, sat 0-100
        String cmd = "{\"method\":\"set_device_info\",\"params\":{" +
                     wrap_param("hue", String(hue), true) + "," +
                     wrap_param("saturation", String(saturation), true) + "," +
                     wrap_param("color_temp", "0", true) + "}}";
        String exp = "\"hue\":" + String(hue);
        return send_command(cmd, exp);
    }

    bool set_color_temperature(uint16_t kelvin) { // e.g. 2500-6500
        String cmd = "{\"method\":\"set_device_info\",\"params\":{" +
                     wrap_param("color_temp", String(kelvin), true) + "," +
                     wrap_param("hue", "0", true) + "," +
                     wrap_param("saturation", "0", true) + "}}";
        String exp = "\"color_temp\":" + String(kelvin);
        return send_command(cmd, exp);
    }

    bool set_hue_saturation(uint16_t hue, uint8_t saturation) {
        return set_color(hue, saturation);
    }
};
