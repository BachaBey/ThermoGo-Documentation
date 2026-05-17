# Hardware — ESP32 Firmware

This folder contains the embedded firmware for the ThermoGo device, built with **PlatformIO** (Arduino framework) targeting the **ESP32-WROOM-32**.

## Contents

```
Hardware/
├── src/
│   └── main.cpp          # Main firmware entry point
├── include/
│   └── config.h          # Pin definitions and constants
├── platformio.ini         # PlatformIO project configuration
└── README.md
```

## Microcontroller

**ESP32-WROOM-32**
- Dual-core 240 MHz Xtensa LX6
- Built-in Bluetooth 4.2 (BLE) and Wi-Fi
- 520 KB SRAM, 4 MB Flash

## Key Libraries

| Library | Purpose |
|---|---|
| `MLX90614` | IR temperature sensor driver (I2C) |
| `Adafruit SSD1306` | OLED 128×64 display |
| `NimBLE-Arduino` | Lightweight BLE GATT server |
| `ArduinoJson` | JSON serialization for BLE packets |

## Build & Flash

```bash
# Install PlatformIO
pip install platformio

# Build firmware
pio run

# Flash to ESP32 (USB connected)
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor
```

## BLE Data Packet

The device sends temperature readings as a JSON notification over BLE:

```json
{
  "temp_c": 36.8,
  "temp_f": 98.2,
  "timestamp": 1715000000,
  "status": "normal"
}
```

Status values: `normal` · `fever` · `high_fever` · `hypothermia`

## Pin Map

| ESP32 Pin | Connected To |
|---|---|
| GPIO 21 (SDA) | MLX90614 + OLED (shared I2C) |
| GPIO 22 (SCL) | MLX90614 + OLED (shared I2C) |
| GPIO 0 | Trigger button (pull-up) |
| 3.3V / GND | All peripherals |
