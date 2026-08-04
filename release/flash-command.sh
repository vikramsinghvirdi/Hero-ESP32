#!/bin/zsh
set -euo pipefail

release_dir=${0:A:h}
port=${1:-}
if [[ -z "$port" ]]; then
  ports=(/dev/cu.usbmodem*(N) /dev/cu.usbserial*(N))
  if (( ${#ports} != 1 )); then
    print -u2 "Expected exactly one ESP32 serial port; found ${#ports}. Pass it as argument 1."
    exit 2
  fi
  port=$ports[1]
fi

python3 -m esptool --chip esp32s3 -p "$port" -b 460800 \
  --before default-reset --after hard-reset write-flash \
  --flash-mode dio --flash-freq 80m --flash-size 8MB \
  0x0 "$release_dir/bootloader.bin" \
  0x8000 "$release_dir/partition-table.bin" \
  0xd000 "$release_dir/ota_data_initial.bin" \
  0x20000 "$release_dir/hero-xiao-esp32s3-sense.bin" \
  0x600000 "$release_dir/generated_assets.bin"
