# Flash Hero firmware on macOS

## Required wiring before power-up

Use 3.3 V for the OLED and 5 V for the MAX98357A amplifier. All grounds must be common.

| From | To XIAO pin | ESP32-S3 GPIO |
|---|---|---:|
| OLED SDA | D4 | 5 |
| OLED SCL | D5 | 6 |
| OLED VCC | 3V3 | — |
| OLED GND | GND | — |
| MAX98357A BCLK/BCK | D1 | 2 |
| MAX98357A DIN | D3 | 4 |
| MAX98357A LRC/WS | D8 | 7 |
| MAX98357A VIN | 5V | — |
| MAX98357A GND | GND | — |
| Speaker lead 1 | MAX98357A SPK+ | — |
| Speaker lead 2 | MAX98357A SPK- | — |

Do not connect either speaker lead to ground. The Sense expansion board microphone is internal:
PDM DATA is GPIO41 and PDM CLK is GPIO42, so it needs no jumper wire.

## Verify and flash

Install esptool in a Python environment (`python3 -m pip install esptool`), connect exactly one
XIAO by USB, then verify the package:

```zsh
cd release
shasum -a 256 -c SHA256SUMS.txt
./erase-and-flash.sh
```

To flash without erasing saved Wi-Fi/activation settings, run `./flash-command.sh`. Either script
accepts an explicit port such as `/dev/cu.usbmodem1101` as its first argument.

The scripts use ESP-IDF's generated layout exactly:

| Offset | Artifact |
|---:|---|
| `0x000000` | `bootloader.bin` |
| `0x008000` | `partition-table.bin` |
| `0x00d000` | `ota_data_initial.bin` |
| `0x020000` | `hero-xiao-esp32s3-sense.bin` |
| `0x600000` | `generated_assets.bin` |

Secure boot, flash encryption, and eFuse changes are not part of either script.
