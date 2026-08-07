#!/bin/sh
# Activate an existing ESP-IDF installation without storing a machine-specific path.

if command -v idf.py >/dev/null 2>&1; then
    idf.py --version
    return 0 2>/dev/null || exit 0
fi

if [ -z "${IDF_PATH:-}" ] || [ ! -f "${IDF_PATH}/export.sh" ]; then
    echo "Set IDF_PATH to an ESP-IDF v6.0.2 installation, then source this file." >&2
    return 1 2>/dev/null || exit 1
fi

. "${IDF_PATH}/export.sh"
idf.py --version
