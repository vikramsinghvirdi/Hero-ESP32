# Hero XIAO ESP32-S3 Sense Test Report

Test date: 2026-08-03 (America/Los_Angeles)

## Automated results

| Check | Result | Evidence |
|---|---|---|
| Chip identity | PASS | ESP32-S3 QFN56 revision 0.2, USB Serial/JTAG |
| Physical flash | PASS | esptool/runtime both reported 8,388,608 bytes |
| PSRAM | PASS | runtime reported 8,388,608 bytes octal PSRAM |
| Flash erase | PASS | full chip erase completed in 5.2 seconds |
| Diagnostic flash | PASS | every written region passed SHA verification |
| PDM microphone | PASS | 30 PCM windows; RMS/peak changed in 27 transitions |
| Speaker I2S write | PASS (electrical/software path) | low-volume 440 Hz buffer accepted |
| OLED I2C probe | BLOCKED | no ACK at 0x3C or 0x3D; OLED was not connected |
| Diagnostic build | PASS | 0x1c07d0 bytes, 40% app-slot margin |
| Normal release build | PASS | 0x27d1a0 bytes, 0x72e60 bytes / 15% app-slot margin |
| Wi-Fi provisioning | NOT RUN | gated on completed OLED/speaker physical diagnostic |
| Xiaozhi activation | NOT RUN | gated on completed hardware diagnostic and Wi-Fi setup |

## Physical verification still required

The diagnostic firmware remains on the device. After wiring the OLED and MAX98357A, reset the
XIAO. Confirm that the OLED shows animated Hero eyes and that the short tone is audible. Serial
should change from `DIAG FAIL OLED` to `DIAG PASS OLED detected at 0x3C` or `0x3D`.

The normal release is built and packaged but intentionally not flashed yet: the handover requires
display and audible-speaker verification before normal provisioning/activation. No unverified
item is marked complete.
