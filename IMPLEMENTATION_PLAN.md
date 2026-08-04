# Hero XIAO ESP32-S3 Sense Implementation Plan

## Pinned upstream

- Repository: `https://github.com/78/xiaozhi-esp32`
- Commit: `66bf9f7c1dece87cb23ab2b9d23d05e7744a9c9b`
- Upstream version at pin: 2.4.1
- ESP-IDF target: 6.0.2 (the upstream preferred stable SDK)
- Custom board identity: `hero-xiao-esp32s3-sense`

## Reference implementations

- `main/boards/bread-compact-wifi`: Wi-Fi board, I2C monochrome OLED, provisioning, buttons, and codec-free I2S audio.
- `main/audio/codecs/no_audio_codec.{h,cc}`: `NoAudioCodecSimplexPdm` provides standard-I2S speaker output plus PDM microphone input on separate ESP32-S3 I2S controllers.
- `main/display/oled_display.{h,cc}`: LVGL monochrome display integration and ESP LCD panel ownership/lifecycle.
- `main/boards/espressif/esp-hi/adc_pdm_audio_codec.*`: additional upstream PDM initialization and diagnostic reference.
- Official Seeed XIAO ESP32-S3 Sense documentation and schematic: PDM DATA GPIO41, PDM CLK GPIO42; XIAO D4/D5 are GPIO5/GPIO6 and D1/D3/D8 are GPIO2/GPIO4/GPIO7.

## Expected board files

- `main/boards/hero-xiao-esp32s3-sense/config.h`: all GPIO assignments and hardware constants.
- `main/boards/hero-xiao-esp32s3-sense/config.json`: unique board/build identity and ESP32-S3 configuration.
- `main/boards/hero-xiao-esp32s3-sense/hero_xiao_esp32s3_sense.cc`: board initialization and hardware validation.
- `main/boards/hero-xiao-esp32s3-sense/hero_eye_display.{h,cc}`: non-blocking procedural monochrome face and state mapping.
- `main/boards/hero-xiao-esp32s3-sense/README.md`: wiring, build, diagnostic, and operation notes.
- Build-system selections in `main/Kconfig.projbuild` and `main/CMakeLists.txt`.
- A project-level diagnostic configuration and release/package documentation.

## Display approach

- Probe I2C addresses `0x3C` and `0x3D` at startup and log the detected address.
- Initialize a 128x128 SH1107 through the ESP LCD I2C panel interface. If the current ESP-IDF component set has no suitable SH1107 driver, add a small source-controlled panel driver based only on the public SH1107 command set.
- Use LVGL monochrome drawing primitives for original Macintosh-style eyes; do not include MakerWorld assets.
- Run animations from an LVGL timer at a conservative frame rate (target 12-15 FPS), with randomized 3-7 second idle blinks and occasional glances.
- Map upstream assistant states to Booting, Idle, Listening, Thinking, Speaking, Offline, and Error without blocking audio/network tasks.
- Show a visible error expression when neither supported OLED address responds, while also emitting an explicit serial FAIL message (serial remains the fallback when the panel is absent).

## Microphone approach

- Reuse upstream `NoAudioCodecSimplexPdm` with PDM clock GPIO42 and data GPIO41.
- Input rate: 16 kHz unless the pinned upstream pipeline requires a different supported rate.
- Diagnostic mode will sample microphone PCM and report RMS/peak periodically; PASS requires changing signal levels rather than merely successful driver initialization.

## Speaker approach

- Reuse upstream codec-free I2S output for MAX98357A: BCLK GPIO2, DOUT GPIO4, WS/LRCK GPIO7.
- Output rate: 24 kHz, matching upstream defaults.
- Diagnostic mode will play a short low-amplitude tone and report driver/write success. Audibility requires one user physical observation.

## Partition and memory approach

- Configure expected 8 MB flash and octal PSRAM for the common XIAO ESP32-S3 Sense module, but treat both as runtime-verified assumptions.
- Use upstream `partitions/v2/8m.csv` if the built application/assets fit with safe margin and its OTA/NVS layout meets Xiaozhi requirements.
- Use generated ESP-IDF flash arguments as the only source of binary offsets.
- Before packaging, confirm physical flash with `esptool flash-id` and runtime flash/PSRAM reports. Do not customize partitions unless the upstream 8 MB layout is insufficient.

## Diagnostic and release sequence

1. Add a compile-time/menuconfig diagnostic mode.
2. Build from a full clean configuration for ESP32-S3.
3. Detect the connected `/dev/cu.*` port, query chip/flash, erase, flash, and monitor.
4. Confirm serial PASS/FAIL results and request only the required display/tone physical observation.
5. Build and flash the normal Xiaozhi firmware, verify provisioning/activation logs, then package genuine build outputs and exact flash scripts.
6. Produce security review, endpoint/credential searches, clean upstream diff, checksums, wiring/build docs, and a validation report.

## Known uncertainties to resolve by inspection or test

- Exact SH1107 initialization sequence, RAM addressing offset, and mirror settings for the user's particular 128x128 module.
- OLED address (`0x3C` or `0x3D`).
- Physical board flash/PSRAM capacity and PSRAM mode; verify rather than relying on product-family defaults.
- MAX98357A channel-slot selection and resulting volume for the wired module.
- Whether GPIO0/BOOT remains the most practical interaction input while USB serial/JTAG is enabled.
- Whether the selected upstream wake-word model fits the final 8 MB image with sufficient OTA margin; use a supported default or button interaction if not.
- Whether a complete activation can be verified without the user's account-side confirmation.
