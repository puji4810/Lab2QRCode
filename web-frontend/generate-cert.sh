#!/bin/bash

# 获取脚本所在目录
BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
CERT_DIR="$BASE_DIR/certs"
mkdir -p "$CERT_DIR"
# 获取本机 IP (支持 Linux 和 macOS)
if [[ "$OSTYPE" == "darwin"* ]]; then
  LOCAL_IP=$(ipconfig getifaddr en0 || ipconfig getifaddr en1)
else
  # Linux 兼容写法
  LOCAL_IP=$(hostname -I | awk '{print $1}')
  if [ -z "$LOCAL_IP" ]; then
    LOCAL_IP=$(ip addr | grep 'state UP' -A2 | grep 'inet ' | awk '{print $2}' | cut -f1 -d'/' | head -n1)
  fi
fi

# 如果还是没拿到 IP，降级到 localhost
if [ -z "$LOCAL_IP" ]; then
  LOCAL_IP="127.0.0.1"
  SAN_EXT="DNS:localhost,IP:127.0.0.1"
else
  SAN_EXT="DNS:localhost,IP:127.0.0.1,IP:$LOCAL_IP"
fi

echo "Detected IP: $LOCAL_IP"

# 生成证书
openssl req -x509 -newkey rsa:4096 -nodes \
  -keyout "$CERT_DIR/key.pem" \
  -out "$CERT_DIR/cert.pem" \
  -days 365 \
  -subj "/C=CN/ST=State/L=City/O=Lab2QRCode/CN=Lab2QRCode-Web" \
  -addext "subjectAltName=$SAN_EXT"

echo ""
echo "Certificate generated successfully!"
echo "IP: $LOCAL_IP"
echo "Location: $CERT_DIR"
echo ""
echo "Now you can run: python3 serve-https.py"
