# Hero Firmware Security Review

Review date: 2026-08-07 (America/Los_Angeles)

## Scope and provenance

- Official upstream: `https://github.com/78/xiaozhi-esp32.git`
- Pinned commit: `66bf9f7c1dece87cb23ab2b9d23d05e7744a9c9b` (version 2.4.1)
- Upstream license: MIT. The Hero board code is original source in this repository; no MakerWorld
  image, offset, asset, reverse engineering, or decompiled code was used.
- ESP-IDF components are resolved at the exact versions and hashes in `dependencies.lock`.
  `espressif/esp_lcd_sh1107` 1.2.0 is the only new component dependency.

## Network behavior

The only compiled-in production service URL found is the upstream OTA/activation endpoint
`https://api.tenclass.net/xiaozhi/ota/` (`CONFIG_OTA_URL`). That service returns the selected
MQTT or WebSocket endpoint and short-lived connection credentials. The source contains many URLs
in documentation and other board definitions; these are not Hero runtime endpoints.

When a conversation audio channel is opened, encoded microphone audio leaves the device through
the server-selected WebSocket or MQTT/UDP transport. Audio is therefore not local-only. There is
no separate telemetry or analytics client in the inspected Hero runtime path. Device ID, client
UUID, board/version information, language, and hardware/system information are sent during the
OTA/activation request.

## Credentials and storage

- Wi-Fi SSIDs/passwords are saved in the `wifi` NVS namespace by `esp-wifi-connect`.
- WebSocket bearer tokens and MQTT username/password/endpoint values returned by activation are
  saved through the Xiaozhi `Settings` NVS wrapper.
- No production API key, password, bearer token, private key, or device credential is embedded in
  the Hero source or release configuration.
- NVS is plaintext at rest because flash encryption is disabled. Anyone with physical flash-read
  access may recover stored credentials.

## Transport security

- The initial OTA/activation URL is HTTPS.
- The Wi-Fi ESP transport attaches ESP-IDF's trusted root certificate bundle for TLS and secure
  MQTT. The release enables the full certificate bundle and cross-signed verification.
- Server-provided endpoints control whether subsequent WebSocket/MQTT transports are secure. The
  official service is expected to supply secure endpoints, but this firmware does not pin a
  server certificate or restrict returned endpoint hostnames.
- OTA downloads use the URL returned by the activation service and write the inactive OTA slot;
  ESP-IDF validates the image format at `esp_ota_end`, but secure-boot signature enforcement is
  not enabled.

## Firmware and device security state

- Secure boot: **disabled** (`CONFIG_SECURE_BOOT` not set).
- Flash encryption: **disabled** (`CONFIG_SECURE_FLASH_ENC_ENABLED` not set).
- No eFuses were changed or requested.
- Debug serial logging is enabled. It exposes device identifiers and network endpoints. Upstream
  debug paths should be reviewed before deploying in an untrusted physical environment.
- Dual OTA slots are present. The English wake-word release uses 0x2b0000-byte app slots and an
  enlarged 0x280000-byte assets partition. The current app leaves 0x323a0 bytes (7%) in each app
  slot.

## Search record and findings

The review used ripgrep equivalents of the requested searches, excluding generated build output:

```text
rg -n -i '(https?|wss?)://' main components docs README*.md
rg -n -i '(api[_-]?key|secret|password|passwd|token|credential|telemetry|analytics)' main components
rg -n 'CONFIG_SECURE_BOOT|CONFIG_SECURE_FLASH_ENC|CONFIG_MBEDTLS_CERTIFICATE_BUNDLE' sdkconfig*
```

Findings were the expected OTA URL, runtime credential field names, documentation links, and
third-party test/example credentials inside downloaded component sources. No suspicious Hero
endpoint, embedded production secret, or independent analytics integration was found. One
upstream BluFi diagnostic path can log a received Wi-Fi password; BluFi is not selected for this
Hero release, but the log should be removed before enabling that provisioning mode.

## Recommendations

1. Self-host a compatible Xiaozhi backend and change `CONFIG_OTA_URL` to a controlled HTTPS host.
2. Allow-list returned WebSocket/MQTT/OTA schemes and hosts; reject plaintext production URLs.
3. Remove credential-bearing/debug logs and lower production log verbosity.
4. After hardware and recovery workflows are mature, evaluate NVS/flash encryption and signed
   secure boot. Those irreversible eFuse operations were intentionally not performed.
