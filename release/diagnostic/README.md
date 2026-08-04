# Hero hardware diagnostic image

This is the exact diagnostic image currently flashed on the tested XIAO. It checks 8 MB flash,
8 MB PSRAM, probes OLED addresses 0x3C/0x3D on GPIO5/GPIO6, emits a low-volume 440 Hz speaker
tone on GPIO2/GPIO4/GPIO7, and measures the internal PDM microphone on GPIO41/GPIO42.

The normal firmware in the parent directory is the release build. Keep this diagnostic image for
hardware troubleshooting; it intentionally does not start Wi-Fi or connect to Xiaozhi.
