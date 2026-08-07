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

A breadboard is not required. Use direct jumper wires or soldered connections. A breadboard is
only an optional mechanical holder or power/ground distribution point. Never join the OLED's
3.3 V rail to the amplifier's 5 V rail.

## Build

```bash
python scripts/build.py hero-xiao-esp32s3-sense \
  --name hero-xiao-esp32s3-sense --language en-US
```

The normal build uses the compact English MultiNet model for the offline phrase **“Hey Hero”**.
The BOOT button remains available as a fallback. `CONFIG_CUSTOM_WAKE_WORD` contains the MultiNet5
phoneme sequence generated for the spoken phrase; the user-facing phrase remains `Hey Hero`.
The 8 MB partition layout retains both OTA application slots and expands the transparent assets
partition to hold the English model and built-in font.

The wake phrase is recognized locally, but ASR, LLM inference, and TTS responses are provided by
the configured network service. Every newly erased device must be provisioned and activated
separately; credentials and activation state are not compiled into the firmware.

While idle, the procedural eyes randomly rotate among look-around, wink, curious, and sleepy
sequences, with randomized blinks between sequences. Listening uses small pupils with a focused
inward orbit and a pulsing mouth. Speaking changes gaze and individual eye height every 160 ms,
making it deliberately more active than idle. No proprietary animation assets are used.
