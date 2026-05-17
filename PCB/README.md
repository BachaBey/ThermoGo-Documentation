# PCB — Altium Schematics and Layouts

This folder contains all PCB design files for the ThermoGo device, created in **Altium Designer**.

## Contents

```
PCB/
├── Schematic/
│   └── ThermoGo_Schematic.SchDoc   # Full circuit schematic
├── PCB_Layout/
│   └── ThermoGo_PCB.PcbDoc         # 2-layer PCB layout
├── Gerbers/
│   ├── ThermoGo.GTL                # Top copper layer
│   ├── ThermoGo.GBL                # Bottom copper layer
│   ├── ThermoGo.GTS                # Top solder mask
│   ├── ThermoGo.GBS                # Bottom solder mask
│   ├── ThermoGo.GTO                # Top silkscreen
│   ├── ThermoGo.GTD                # Top drill file
│   └── ThermoGo.TXT                # NC drill file
├── BOM.csv                         # Bill of Materials
└── README.md
```

## Board Specifications

| Parameter | Value |
|---|---|
| Layers | 2 (Top + Bottom copper) |
| Board dimensions | ~75mm × 35mm |
| Minimum trace width | 0.2mm |
| Minimum via size | 0.6mm drill / 1.0mm pad |
| Surface finish | HASL (lead-free) |
| Validated with | JLCPCB DFM rules |

## Bill of Materials (Key Components)

| Reference | Component | Value / Part Number |
|---|---|---|
| U1 | ESP32-WROOM-32 | Espressif ESP32-WROOM-32 |
| U2 | IR Temperature Sensor | MLX90614ESF-BAA |
| U3 | LiPo Charger IC | TP4056 |
| U4 | Voltage Regulator | AMS1117-3.3 |
| DISP1 | OLED Display | SSD1306 128×64 0.96" |
| C1–C4 | Decoupling Capacitors | 100nF / 10µF |
| R1–R2 | I2C Pull-ups | 4.7kΩ |
| J1 | USB-C Connector | USB Type-C 2.0 |
| SW1 | Trigger Button | 6×6mm Tactile |
| BAT1 | LiPo Battery | 3.7V 1000mAh |

## Design Notes

- Single I2C bus (GPIO 21/22) shared between MLX90614 and OLED — addresses are distinct (0x5A and 0x3C respectively)
- TP4056 wired to USB-C for safe single-cell LiPo charging with over-discharge protection
- ESP32 BOOT (GPIO 0) and EN pins exposed as test pads for serial flashing
- Power LED (green) and charging status LED (red/blue) included

## Manufacturing

Gerber files in `Gerbers/` are ready for fabrication. Upload the zip of the Gerbers folder directly to JLCPCB, PCBWay, or equivalent.
