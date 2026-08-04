# Hero XIAO ESP32-S3 Sense

Trusted, source-controlled Xiaozhi client firmware for the Seeed Studio XIAO ESP32-S3 Sense.

## Wiring

| Peripheral | Signal | ESP32-S3 GPIO | XIAO label |
|---|---|---:|---|
| SH1107 OLED | SDA | 5 | D4 |
| SH1107 OLED | SCL | 6 | D5 |
| SH1107 OLED | VCC | 3V3 | 3V3 |
| SH1107 OLED | GND | GND | GND |
| MAX98357A | BCLK | 2 | D1 |
| MAX98357A | DIN | 4 | D3 |
| MAX98357A | LRC/WS | 7 | D8 |
| MAX98357A | VIN | 5V | 5V |
| MAX98357A | GND | GND | GND |
| Sense microphone | PDM DATA | 41 | internal |
| Sense microphone | PDM CLK | 42 | internal |

Connect the speaker only across the MAX98357A `SPK+` and `SPK-` outputs. Never connect either
speaker lead to ground.

The firmware probes OLED addresses `0x3C` and `0x3D`. BOOT enters provisioning while the device
is starting and toggles conversation afterward.

## Build

```bash
python scripts/build.py hero-xiao-esp32s3-sense --language en-US --wake-word disabled
```

The first hardware build intentionally uses button interaction. A supported ESP-SR wake word can
be selected later after flash/PSRAM and application-size validation.
