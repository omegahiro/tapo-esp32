# Tapo-ESP32

Unofficial Tapo API Client for ESP32.

## Device support

- Supports Tapo P-series smart plugs and L-series smart lights.
- Compatible with the latest Tapo firmware that uses the KLAP algorithm.
- (Not supports Passthrough algorithm used in the older firmwares.)
- Simple, lightweight, and easy-to-use API.

## Prerequisites

- Arduino IDE or PlatformIO.

## Installation

Copy the following files into your project:
- `tapo_device.h`
- `tapo_protocol.h`
- `tapo_cipher.h`

## Example

```cpp
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
```

## Credits

Inspired by [mihai-dinculescu/tapo][inspired_by].

[inspired_by]: https://github.com/mihai-dinculescu/tapo
