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

You can either use this project directly as a sketch, or copy the library files into your own project.

### Option 1: Use as a sketch (Arduino IDE)

1. Download or clone this repository.
2. Open `tapo-esp32.ino` by double-clicking it (or via Arduino IDE).
3. Edit Wi-Fi and Tapo credentials, then upload.

### Option 2: Copy files into your project

Copy these files into your project:

- `tapo_device.h`
- `tapo_protocol.h`
- `tapo_cipher.h`

## Example

- [tapo-esp32.ino](tapo-esp32.ino)

## Contributing

Contributions are welcome.

## Credits

Inspired by [mihai-dinculescu/tapo](https://github.com/mihai-dinculescu/tapo)
