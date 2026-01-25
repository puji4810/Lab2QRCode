#!/bin/bash
# Lab2QRCode Web - Quick Start Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "╔══════════════════════════════════════════════════════╗"
echo "║  Lab2QRCode Web - Quick Start                        ║"
echo "╚══════════════════════════════════════════════════════╝"
echo ""

if [ ! -d "node_modules" ]; then
    echo "📦 Installing dependencies..."
    npm install
    echo ""
fi

if [ ! -d "dist" ]; then
    echo "🔨 Building production bundle..."
    npm run build
    echo ""
fi

echo "🚀 Starting server..."
echo ""
python3 serve.py
