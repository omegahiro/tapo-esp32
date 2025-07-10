#include <Arduino.h>
#include <WiFi.h>

#define TAPO_DEBUG_MODE // Comment this line to disable debug messages
#include "tapo_device.h"

TapoDevice tapo;

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin("wifi-ssid", "wifi-password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    // Initialize Tapo device
    tapo.begin("device-ip-address", "tapo-username", "tapo-password");
    // Example: tapo.begin("192.168.1.100", "abc@example.com", "abc123");
}

void loop() {
    tapo.on(); // Turn on the device
    delay(5000);
    tapo.off(); // Turn off the device
    delay(5000);
}