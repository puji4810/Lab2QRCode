# Lab2QRCode Web Application

基于 WebAssembly 的条码生成和识别 Web 应用。

## 快速开始

### 方法一：一键启动（推荐）

```bash
cd web-frontend
./start.sh
```

然后在浏览器中打开：
- **http://localhost:8080** (本机访问)
- **http://192.168.9.100:8080** (局域网访问，替换为你的实际IP)

### 方法二：手动启动

#### 1. 安装依赖

```bash
cd web-frontend
npm install
```

#### 2. 开发模式（推荐用于开发）

```bash
npm run dev
```

访问 http://localhost:3000

#### 3. 生产模式（推荐用于部署）

**构建：**

```bash
npm run build
```

**启动服务器：**

```bash
python3 serve.py
```

访问 http://localhost:8080

## 项目结构

```
Lab2QRCode/
├── wasm-module/              # WASM 模块源码
│   ├── src/
│   │   └── barcode_api_simple.cpp
│   ├── build/
│   │   ├── lab2qrcode.js     # WASM 绑定 (64KB)
│   │   └── lab2qrcode.wasm   # WASM 二进制 (944KB)
│   └── CMakeLists.txt
├── wasm-deps/                # WASM 依赖库
│   └── zxing-cpp/
└── web-frontend/             # React 前端
    ├── src/
    │   ├── components/       # React 组件
    │   ├── hooks/            # React Hooks
    │   ├── utils/            # 工具函数
    │   └── types/            # TypeScript 类型定义
    ├── public/               # 静态资源
    │   ├── lab2qrcode.js
    │   └── lab2qrcode.wasm
    ├── dist/                 # 构建输出
    ├── serve.py              # 生产服务器
    └── start.sh              # 快速启动脚本
```

## 功能特性

### ✅ 已实现

- **条码生成**：支持 20 种条码格式
- **条码解码**：从图片文件解码条码
- **相机扫描**：使用设备摄像头实时扫描（需 HTTPS 或 localhost）
- **Base64 编解码**：支持二进制数据处理
- **SVG 输出**：矢量图形格式，无损缩放

### 🎯 支持的条码格式

| 类型 | 格式 |
|------|------|
| **二维码** | QRCode, MicroQRCode, RMQRCode, Aztec, DataMatrix, PDF417, MaxiCode |
| **一维码** | Code128, Code39, Code93, Codabar, EAN8, EAN13, UPCA, UPCE, ITF, DataBar, DataBarExpanded, DataBarLimited, DXFilmEdge |

## 重新编译 WASM 模块

如果修改了 C++ 代码，需要重新编译 WASM 模块：

```bash
cd wasm-module/build
emmake make -j$(nproc)
cp lab2qrcode.{js,wasm} ../../web-frontend/public/
cd ../../web-frontend
npm run build
```

## 命令速查表

| 命令 | 说明 |
|------|------|
| `./start.sh` | 一键启动服务器（推荐） |
| `npm run dev` | 启动开发服务器 |
| `npm run build` | 构建生产版本 |
| `python3 serve.py` | 启动生产服务器 |
| `pkill -f "vite\|python3.*serve"` | 停止所有服务器 |

## 浏览器要求

- Chrome/Edge 90+
- Firefox 90+
- Safari 15+

相机功能需要 HTTPS 或 localhost 环境。

## 故障排查

### 无法访问 3001/8080 端口

**原因：** 防火墙阻止或端口被占用

**解决：**

```bash
# 检查端口占用
ss -tlnp | grep 8080

# 检查服务器进程
ps aux | grep -E "vite|python3.*serve"

# 重启服务器
pkill -f "vite\|python3.*serve"
./start.sh
```

### WASM 模块加载失败

**原因：** WASM 文件不存在或路径错误

**解决：**

```bash
# 确认文件存在
ls -lh web-frontend/dist/lab2qrcode.*

# 重新复制 WASM 文件
cp wasm-module/build/lab2qrcode.{js,wasm} web-frontend/public/
npm run build
```

### 相机无法启动

**原因：** 浏览器安全限制

**解决：**

1. 确保使用 HTTPS 或 localhost
2. 检查浏览器相机权限
3. 在浏览器设置中允许网站访问相机

## 技术栈

- **前端框架**: React 18 + TypeScript
- **构建工具**: Vite 5
- **WASM 编译**: Emscripten
- **条码库**: ZXing-cpp v2.3.0
- **样式**: CSS Modules

## License

MIT
