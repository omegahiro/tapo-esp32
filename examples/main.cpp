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
    // Example: tapo.begin("192.168.1.100", "abc@example.com", "abc123");
    while (!tapo.begin("device-ip-address", "tapo-username", "tapo-password")) {
        Serial.println("Tapo init failed. Retrying in 2s...");
        delay(2000);
    }
}

void loop() {
    // === Common controls (Plugs & Lights) ===
    tapo.off(); // Turn the device OFF
    delay(3000);
    tapo.on(); // Turn the device ON
    delay(3000);

    // === Light-specific controls ===
    tapo.set_brightness(100);          // Set brightness (1–100%)
    tapo.set_color(0, 100);            // Set color: hue (0–360°), saturation (0–100%)
    tapo.set_color_temperature(2500);  // Set color temperature in Kelvin (e.g., 2500–6500)
    tapo.set_hue_saturation(120, 100); // Same as set_color()
}
