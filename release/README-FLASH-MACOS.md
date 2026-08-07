# Flash the ready-to-use Hero firmware

This directory is the binary package for people who want to assemble an identical Hero without
installing ESP-IDF or compiling source. The full source remains in the parent repository.

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

## Simplest method: one complete binary

`hero-xiao-esp32s3-sense-full.bin` is a complete 8 MB image. It already contains every required
region at the correct address. Flash it at `0x0`:

```zsh
python3 -m pip install --user esptool
python3 -m esptool --chip esp32s3 -p /dev/cu.usbmodem101 erase-flash
python3 -m esptool --chip esp32s3 -p /dev/cu.usbmodem101 -b 460800 \
  --before default-reset --after hard-reset write-flash 0x0 \
  hero-xiao-esp32s3-sense-full.bin
```

Replace `/dev/cu.usbmodem101` with the connected port. The full erase deletes old Wi-Fi and
activation data. Do not substitute the smaller application-only
`hero-xiao-esp32s3-sense.bin` at address `0x0`.

## Alternative method: individual binaries

Install esptool in a Python environment (`python3 -m pip install --user esptool`), connect exactly
one XIAO with a data-capable USB cable, then verify the package:

```zsh
cd release
shasum -a 256 -c SHA256SUMS.txt
./erase-and-flash.sh
```

To flash without erasing saved Wi-Fi/activation settings, run `./flash-command.sh`. Either script
accepts an explicit port such as `/dev/cu.usbmodem101` as its first argument.

The scripts write the same content as the full image using ESP-IDF's generated layout:

| Offset | Artifact |
|---:|---|
| `0x000000` | `bootloader.bin` |
| `0x008000` | `partition-table.bin` |
| `0x00d000` | `ota_data_initial.bin` |
| `0x020000` | `hero-xiao-esp32s3-sense.bin` |
| `0x580000` | `generated_assets.bin` |

The assets image contains the English ESP-SR model required by the offline “Hey Hero” detector.
NVS is deliberately absent from this package, so it contains no saved Wi-Fi password, activation
state, MAC address, or device UUID.

Secure boot, flash encryption, and eFuse changes are not part of either script.
