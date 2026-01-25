#!/bin/bash

# Lab2QRCode WASM 依赖编译脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/wasm-deps"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "========================================="
echo "编译 ZXing-cpp for WebAssembly"
echo "========================================="

if [ ! -d "zxing-cpp" ]; then
    git clone --depth 1 --branch v2.3.0 https://github.com/zxing-cpp/zxing-cpp.git
fi

cd zxing-cpp
mkdir -p build-wasm
cd build-wasm

echo "配置 ZXing..."
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_BLACKBOX_TESTS=OFF \
    -DBUILD_UNIT_TESTS=OFF \
    -DBUILD_PYTHON_MODULE=OFF

echo "编译 ZXing..."
emmake make -j$(nproc)

echo "安装 ZXing..."
DESTDIR="$BUILD_DIR/install" emmake make install

echo ""
echo "========================================="
echo "编译完成！"
echo "========================================="
echo "ZXing WASM 库位置:"
echo "  $BUILD_DIR/install"
echo ""
echo "现在可以编译 Lab2QRCode WASM 模块了："
echo "  cd $SCRIPT_DIR/wasm-module/build"
ZXING_CMAKE_DIR=$(find "$BUILD_DIR/install" -name "ZXingConfig.cmake" -exec dirname {} \; | head -1)
echo "  emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -DZXing_DIR=$ZXING_CMAKE_DIR"
echo "  emmake make -j\$(nproc)"
