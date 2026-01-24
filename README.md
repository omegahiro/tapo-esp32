# Tapo-ESP32

Unofficial Tapo API client for ESP32.  
Control Tapo **P-series smart plugs** and **L-series smart lights** from an ESP32.

## Features

- Supports Tapo **P-series (plugs)** and **L-series (lights)**
- Compatible with the latest Tapo firmware using the **KLAP** algorithm
- Lightweight and simple API

> Note: The **Passthrough** algorithm used in older firmwares is **not supported**. Please update to the latest version.

## Tested environment

- **ESP32-WROOM-32**
- **Tapo P105** firmware **v1.4.5** (latest as of **Jan 2026**)
- **Tapo P110** firmware **v1.4.1** (latest as of **Jan 2026**)
- **Tapo L535** firmware **v1.1.7**

> **Important**: For newer firmware versions, make sure **Third-Party Compatibility** is turned on in the Tapo app. You can find this option by navigating to **Me > Third-Party Services** in the app.

## Prerequisites

- Arduino IDE or PlatformIO

## Installation

Copy these files into your project:

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
```

## Contributing

Contributions are welcome.

## Credits

Inspired by [mihai-dinculescu/tapo](https://github.com/mihai-dinculescu/tapo)
