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
  --before default-reset --after hard-reset erase-flash
"$release_dir/flash-command.sh" "$port"
