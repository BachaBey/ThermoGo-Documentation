# 3D Model — Device Enclosure

This folder contains the CAD files for the ThermoGo handheld enclosure, designed in **Fusion 360** and exported as STEP files for universal compatibility.

## Contents

```
3D Model/
├── ThermoGo_Body.step       # Main enclosure body
├── ThermoGo_Lid.step        # Snap-fit back cover
├── ThermoGo_Assembly.step   # Full assembled model
├── renders/
│   ├── front_view.png       # Front render
│   ├── back_view.png        # Back render
│   └── exploded_view.png    # Exploded assembly view
└── README.md
```

## Enclosure Specifications

| Parameter | Value |
|---|---|
| Dimensions | ~85mm × 40mm × 22mm |
| Material | PLA (recommended) or PETG |
| Print layer height | 0.2mm |
| Infill | 20% (gyroid or grid) |
| Supports needed | No |
| Assembly | Snap-fit lid, no screws |

## Cutouts & Features

| Feature | Detail |
|---|---|
| Sensor window | 10mm circular aperture for MLX90614 |
| OLED display | 26mm × 15mm rectangular window |
| USB-C port | 9mm × 3.5mm slot on bottom edge |
| Trigger button | 8mm circular opening on top face |
| PCB standoffs | 4× M2 brass inserts (heat-set) |

## Printing Instructions

1. Export `.step` files to your slicer (PrusaSlicer, Cura, Bambu Studio)
2. Orient body face-down for best surface finish on the front
3. Print lid flat (no rotation needed)
4. Use 2–3 perimeters for adequate wall strength around snap clips
5. Heat-set M2 brass inserts into the four PCB standoffs before assembly

## Tools Required for Assembly

- Soldering iron (heat-set inserts)
- M2 × 4mm screws (PCB mounting)
- Small flathead screwdriver (lid pry, if needed)
