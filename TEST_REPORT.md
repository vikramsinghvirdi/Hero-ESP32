# Hero XIAO ESP32-S3 Sense Test Report

Test window: 2026-08-03 through 2026-08-04 (America/Los_Angeles)

## Results

| Check | Result | Evidence |
|---|---|---|
| Chip identity | PASS | ESP32-S3 QFN56 revision 0.2, USB Serial/JTAG |
| Physical flash | PASS | esptool/runtime both reported 8,388,608 bytes |
| PSRAM | PASS | runtime reported 8,388,608 bytes octal PSRAM |
| Full flash erase | PASS | full-chip erase completed before initial deployment |
| Written data verification | PASS | esptool verified the SHA digest after flashing |
| PDM microphone | PASS | 30 PCM windows; RMS/peak changed in 27 transitions |
| Speaker | PASS (physical) | 0.7-second 440 Hz diagnostic tone was heard |
| OLED | PASS (physical) | SH1107 detected at `0x3C`; centered animated Hero eyes observed |
| Diagnostic build | PASS | diagnostic firmware built and ran on physical hardware |
| Normal release build | PASS | `0x27dc60` bytes; `0x323a0` bytes / 7% free in each app slot |
| Wi-Fi provisioning | PASS | connected to the selected 2.4 GHz network and obtained DHCP address |
| Xiaozhi activation | PASS | MQTT connection completed and runtime logged `Activation done` |
| English wake model | PASS | `mn5q8_en` loaded and MultiNet initialized with `Hey Hero` command |
| Offline “Hey Hero” wake | PASS | physical phrase detected at probability `0.302938` |
| Listening session | PASS | state advanced `idle -> connecting -> listening` |
| Speech recognition | PASS | device recognized “Tell me a joke” and follow-up requests |
| Cloud response/TTS path | PASS | server returned and device played multiple spoken responses |
| Return to idle | PASS | session received goodbye, closed audio, and returned to idle |
| New focused/speaking gestures | BUILD/RUNTIME PASS | state-driven code built and ran; final subjective appearance remains user-adjustable |

## Physical and conversation verification

After wiring the OLED and MAX98357A, the diagnostic boot detected the display, showed the Hero
face, played an audible test tone, and measured changing microphone levels. The normal firmware
then provisioned Wi-Fi, activated successfully, loaded the offline English model, detected “Hey
Hero,” opened a cloud session, recognized several spoken requests, played the returned audio, and
cleanly closed the session.

The wake word is therefore verified independently of the cloud path: detection happens first on
the ESP32, after which the network conversation begins. A USB docking-station data-path failure
was also reproduced: the OLED remained powered while macOS had no ESP32 serial device. Direct USB
connection restored `/dev/cu.usbmodem*`; this was a dock/cable-path issue rather than a firmware
or wake-model failure.

## Current configuration

- Board: `hero-xiao-esp32s3-sense`
- ESP-IDF: v6.0.2
- Application: Xiaozhi 2.4.1 plus Hero board support
- Wake phrase: `Hey Hero`
- MultiNet phonemes: `hd hgRb`
- Detection threshold: `15`
- Recommended speaker volume: 75%
- Normal assets offset: `0x580000`
- Security operations: no secure boot, flash encryption, or eFuse changes
