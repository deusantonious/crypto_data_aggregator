#!/usr/bin/env bash
set -euo pipefail

if [[ "${EUID}" -ne 0 ]]; then
    echo "Run as root (e.g. with sudo)."
    exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SERVICE_NAME="crypto_data_aggregator"
BINARY_SOURCE="${ROOT_DIR}/build/release/crypto_data_aggregator"
CONFIG_SOURCE="${ROOT_DIR}/config.json"
INSTALL_BIN="/usr/local/bin/${SERVICE_NAME}"
INSTALL_DIR="/etc/${SERVICE_NAME}"
UNIT_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

if [[ ! -f "${BINARY_SOURCE}" ]]; then
    echo "Binary not found at: ${BINARY_SOURCE}"
    echo "Build first: cmake --preset release && cmake --build --preset release -j"
    exit 1
fi

if [[ ! -f "${CONFIG_SOURCE}" ]]; then
    echo "Config file not found at: ${CONFIG_SOURCE}"
    exit 1
fi

install -d -m 755 "${INSTALL_DIR}"
install -m 755 "${BINARY_SOURCE}" "${INSTALL_BIN}"
install -m 644 "${CONFIG_SOURCE}" "${INSTALL_DIR}/config.json"

cat > "${UNIT_FILE}" <<EOF
[Unit]
Description=Binance WebSocket Aggregator
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=${INSTALL_DIR}
ExecStart=${INSTALL_BIN}
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now "${SERVICE_NAME}.service"
systemctl status --no-pager "${SERVICE_NAME}.service"
