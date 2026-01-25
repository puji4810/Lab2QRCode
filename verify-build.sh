#!/bin/bash
# 本地验证脚本 - 模拟 GitHub Actions 的构建流程

set -e

echo "========================================"
echo "Lab2QRCode WASM 本地构建验证"
echo "========================================"

# 颜色输出
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查 Emscripten
echo -e "\n${YELLOW}[1/6] 检查 Emscripten...${NC}"
if command -v emcc &> /dev/null; then
    echo -e "${GREEN}✓ Emscripten 已安装${NC}"
    emcc --version | head -1
else
    echo -e "${RED}✗ Emscripten 未安装${NC}"
    echo "请先安装 Emscripten: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

# 清理旧构建
echo -e "\n${YELLOW}[2/6] 清理旧构建...${NC}"
rm -rf wasm-deps/install wasm-module/build
echo -e "${GREEN}✓ 清理完成${NC}"

# 构建 ZXing
echo -e "\n${YELLOW}[3/6] 构建 ZXing-cpp...${NC}"
chmod +x build-wasm-deps.sh
./build-wasm-deps.sh

# 验证 ZXing 安装
echo -e "\n${YELLOW}[4/6] 验证 ZXing 安装...${NC}"
if [ -d "wasm-deps/install" ]; then
    echo -e "${GREEN}✓ Install 目录存在${NC}"
    ZXING_CONFIG=$(find wasm-deps/install -name "ZXingConfig.cmake" | head -1)
    if [ -n "$ZXING_CONFIG" ]; then
        echo -e "${GREEN}✓ ZXingConfig.cmake 找到:${NC}"
        echo "  $ZXING_CONFIG"
    else
        echo -e "${RED}✗ ZXingConfig.cmake 未找到${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ Install 目录不存在${NC}"
    exit 1
fi

# 构建 WASM 模块
echo -e "\n${YELLOW}[5/6] 构建 WASM 模块...${NC}"
cd wasm-module
mkdir -p build
cd build

ZXING_CMAKE_DIR=$(find ../../wasm-deps/install -name "ZXingConfig.cmake" -exec dirname {} \; | head -1)

if [ -z "$ZXING_CMAKE_DIR" ]; then
    echo -e "${RED}✗ 无法找到 ZXingConfig.cmake${NC}"
    exit 1
fi

echo -e "${GREEN}✓ 使用 ZXing 配置:${NC}"
echo "  $ZXING_CMAKE_DIR"

emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DZXing_DIR="$ZXING_CMAKE_DIR"

emmake make -j$(nproc)

# 验证输出
echo -e "\n${YELLOW}[6/6] 验证输出文件...${NC}"
if [ -f "lab2qrcode.js" ] && [ -f "lab2qrcode.wasm" ]; then
    echo -e "${GREEN}✓ WASM 文件生成成功:${NC}"
    ls -lh lab2qrcode.js lab2qrcode.wasm
else
    echo -e "${RED}✗ WASM 文件生成失败${NC}"
    exit 1
fi

# 复制到前端
echo -e "\n${YELLOW}复制 WASM 文件到前端...${NC}"
cd ../..
cp wasm-module/build/lab2qrcode.js web-frontend/public/
cp wasm-module/build/lab2qrcode.wasm web-frontend/public/
echo -e "${GREEN}✓ 文件已复制到 web-frontend/public/${NC}"

# 成功
echo -e "\n========================================"
echo -e "${GREEN}✓ 构建验证成功！${NC}"
echo "========================================"
echo ""
echo "下一步："
echo "  1. cd web-frontend"
echo "  2. npm install"
echo "  3. npm run build"
echo "  4. python3 serve.py"
echo ""
echo "或者推送到 GitHub 触发自动部署："
echo "  git add ."
echo "  git commit -m 'Update WASM build'"
echo "  git push origin master"
