#pragma once

#include "tapo_protocol.h"

#define TAPO_MAX_SEND_RETRIES 3
#define TAPO_MAX_RECONNECT_RETRIES 3

class TapoDevice {
private:
    TapoProtocol protocol;

    bool check_state(const String &expected_state) {
        String response = protocol.send_message("{\"method\":\"get_device_info\",\"params\":{}}");
        return response.indexOf(expected_state) != -1;
    }

    bool send_command(const String &command, const String &expected_state = "") {
        for (int i = 0; i < TAPO_MAX_RECONNECT_RETRIES; i++) {
            for (int j = 0; j < TAPO_MAX_SEND_RETRIES; j++) {
                if (expected_state == "" || check_state(expected_state)) {
                    return true;
                }
                protocol.send_message(command);
                delay(1000); // Wait for the command to be applied
            }
            Serial.println("TAPO_DEVICE: Failed to send command, rehandshaking...");
            protocol.rehandshake();
        }
        Serial.println("TAPO_DEVICE: Failed to rehandshake, giving up command");
        
        return false;
    }

public:
    void begin(const String& ip_address, const String& username, const String& password) {
        protocol.handshake(ip_address, username, password);
    }

    void on() {
        String command = "{\"method\":\"set_device_info\",\"params\":{\"device_on\":true}}";
        String expected_state = "\"device_on\":true";
        send_command(command, expected_state);
    }

    void off() {
        String command = "{\"method\":\"set_device_info\",\"params\":{\"device_on\":false}}";
        String expected_state = "\"device_on\":false";
        send_command(command, expected_state);
    }
};
