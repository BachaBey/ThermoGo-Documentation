# ThermoGo — Smart Wireless Thermometer

> A portable, Bluetooth-enabled medical thermometer with real-time mobile monitoring, instant alerts, and health history tracking.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Repository Structure](#repository-structure)
- [Hardware](#hardware)
- [Firmware](#firmware)
- [Mobile Application](#mobile-application)
- [PCB Design](#pcb-design)
- [3D Enclosure](#3d-enclosure)
- [Getting Started](#getting-started)
- [Demo](#demo)
- [Team](#team)
- [License](#license)

---

## Overview

**ThermoGo** is a smart, wireless body thermometer designed for home healthcare and clinical support. It pairs with a React Native mobile application via Bluetooth Low Energy (BLE), allowing users to measure body temperature, receive instant fever alerts, and maintain a timestamped health log — all without cables or manual reading.

The device is built around an **ESP32 microcontroller** with an **MLX90614 infrared temperature sensor**, delivering non-contact readings in under one second. The mobile app displays real-time temperature, trends, and notifications for temperatures outside the normal range.

---

## Features

| Feature | Description |
|---|---|
| Non-contact measurement | MLX90614 IR sensor, accurate to ±0.5°C |
| Wireless connectivity | Bluetooth Low Energy (BLE 4.2) via ESP32 |
| Mobile application | React Native app (iOS & Android) |
| Real-time alerts | Push notifications for fever (>37.5°C) or hypothermia (<36°C) |
| Health history | Timestamped log with chart visualization |
| Compact enclosure | Custom 3D-printed housing for ergonomic handheld use |
| Long battery life | 3.7V LiPo battery with USB-C charging |

---

## System Architecture

```
┌─────────────────────────────────────┐
│           ThermoGo Device           │
│                                     │
│  [MLX90614] ──I2C──► [ESP32]        │
│                          │          │
│  [LiPo Battery]          │ BLE      │
│  [USB-C Charger]         │          │
│  [OLED Display]          │          │
└──────────────────────────┼──────────┘
                           │
                    Bluetooth LE
                           │
┌──────────────────────────▼──────────┐
│        ThermoGo Mobile App          │
│        (React Native)               │
│                                     │
│  • Real-time temperature display    │
│  • Fever/hypothermia alerts         │
│  • History log & trend charts       │
│  • User profile management          │
└─────────────────────────────────────┘
```

---

## Repository Structure

```
ThermoGo-Documentation/
│
├── Hardware/               # ESP32 firmware (PlatformIO project)
│   ├── src/                # C++ source files
│   ├── include/            # Header files
│   ├── platformio.ini      # PlatformIO configuration
│   └── README.md
│
├── Software/               # React Native mobile application
│   ├── src/
│   │   ├── screens/        # App screens (Home, History, Settings)
│   │   ├── components/     # Reusable UI components
│   │   ├── services/       # BLE communication layer
│   │   └── navigation/     # App navigation config
│   ├── package.json
│   └── README.md
│
├── 3D Model/               # CAD files for device enclosure
│   ├── ThermoGo_Body.step
│   ├── ThermoGo_Lid.step
│   └── renders/            # Rendered preview images
│
├── PCB/                    # Altium schematics and PCB layouts
│   ├── Schematic/
│   ├── PCB_Layout/
│   ├── Gerbers/            # Ready-to-manufacture Gerber files
│   └── BOM.csv             # Bill of Materials
│
├── Brand/                  # ThermoGo visual identity
│   ├── Logo/               # SVG, PNG variants (light/dark)
│   ├── Colors.md           # Brand color palette
│   └── Typography.md       # Font choices and usage
│
├── Demo Video/             # Product demonstration video
│   └── ThermoGo_Demo.mp4
│
├── Report/                 # Final project report and documentation
│   └── ThermoGo_Report.pdf
│
└── README.md               # This file
```

---

## Hardware

### Components

| Component | Model | Purpose |
|---|---|---|
| Microcontroller | ESP32-WROOM-32 | Main processor + BLE radio |
| Temperature Sensor | MLX90614 (GY-906) | Non-contact IR thermometry |
| Display | SSD1306 OLED 0.96" | Local temperature readout |
| Battery | 3.7V 1000mAh LiPo | Power supply |
| Charging IC | TP4056 | USB-C LiPo charging |
| Voltage Regulator | AMS1117-3.3 | 3.3V regulated supply |
| Tactile Button | 6×6mm | Single-button measurement trigger |

### Wiring Summary

```
ESP32 GPIO 21 (SDA) ──► MLX90614 SDA
ESP32 GPIO 22 (SCL) ──► MLX90614 SCL
ESP32 GPIO 21 (SDA) ──► OLED SDA  (shared I2C bus)
ESP32 GPIO 22 (SCL) ──► OLED SCL
ESP32 GPIO 0        ──► Trigger Button (pull-up)
3.3V / GND          ──► All peripherals
```

---

## Firmware

The ESP32 firmware is built with **PlatformIO** (Arduino framework).

### Key Libraries

- `MLX90614` — IR sensor driver
- `Adafruit SSD1306` — OLED display
- `NimBLE-Arduino` — Lightweight BLE stack
- `ArduinoJson` — JSON serialization for BLE data packets

### Build & Flash

```bash
# Install PlatformIO CLI
pip install platformio

# Navigate to firmware folder
cd Hardware/

# Build
pio run

# Flash to connected ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### BLE Data Format

The device advertises a custom GATT service and sends temperature readings as a JSON string:

```json
{
  "temp_c": 36.8,
  "temp_f": 98.2,
  "timestamp": 1715000000,
  "status": "normal"
}
```

**Status values:** `normal` | `fever` | `high_fever` | `hypothermia`

---

## Mobile Application

The ThermoGo app is built with **React Native** (Expo managed workflow) targeting both iOS and Android.

### Prerequisites

```bash
node >= 18.x
npm >= 9.x
Expo CLI
```

### Setup & Run

```bash
cd Software/

# Install dependencies
npm install

# Start Expo dev server
npx expo start

# Run on Android
npx expo run:android

# Run on iOS
npx expo run:ios
```

### App Screens

| Screen | Description |
|---|---|
| Home | Connect to device, trigger measurement, live temperature display |
| History | Scrollable log of past readings with date/time stamps |
| Chart | Temperature trend graph over time |
| Settings | Alert thresholds, user profile, units (°C / °F) |

### BLE Integration

The app scans for the ThermoGo BLE service UUID, connects automatically when the device is in range, and subscribes to the temperature characteristic notification. All readings are persisted locally using AsyncStorage.

---

## PCB Design

The PCB was designed in **Altium Designer**. The layout is a compact 2-layer board sized to fit inside the 3D-printed enclosure.

### Key Design Decisions

- Single I2C bus shared between the MLX90614 and OLED (address-differentiated)
- On-board USB-C connector wired through TP4056 for safe LiPo charging
- ESP32 BOOT and EN buttons exposed via test pads for programming
- Power LED and charging status indicator LEDs

### Manufacturing Files

Gerber files ready for fabrication are located in `PCB/Gerbers/`. The board was validated with JLCPCB DFM rules.

---

## 3D Enclosure

The enclosure was designed in **Fusion 360** and exported as STEP files for universal compatibility.

- Two-part snap-fit shell (body + lid)
- Cutouts for USB-C port, MLX90614 sensor window, OLED display, and trigger button
- Material: PLA, printed at 0.2mm layer height
- Dimensions: approx. 85mm × 40mm × 22mm

---

## Getting Started

### Full System Setup

1. **Flash the firmware** — follow the [Firmware](#firmware) section.
2. **Power the device** — connect via USB-C to charge, then switch on.
3. **Install the app** — clone the repo, run `npm install` in `Software/`, and launch with Expo.
4. **Pair** — open the app, tap "Scan for Devices", select *ThermoGo* from the list.
5. **Measure** — press the trigger button on the device or the on-screen button in the app.

---

## Demo

A full video demonstration of ThermoGo in action is available in the [Demo Video/](Demo%20Video/) folder.

The demo covers:
- Device power-on and BLE pairing
- Live temperature measurement
- Fever alert notification
- History log and chart view

---

## Team

| Name | Role |
|---|---|
| BachaBey | Project Lead / Firmware Engineer |
| — | Hardware & PCB Design |
| — | Mobile Application Development |
| — | 3D Modelling & Enclosure Design |

> This project was developed as part of the ISS396 Engineering capstone module.

---

## License

This project is released for educational purposes. All design files, firmware, and software are provided as-is. Contact the team before any commercial use.

---

*ThermoGo — measure smarter, live healthier.*
