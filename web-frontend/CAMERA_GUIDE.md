# 🎥 摄像头功能使用指南

## 问题：无法访问摄像头

浏览器安全策略要求摄像头访问必须在以下环境之一：
- ✅ `localhost` 
- ✅ `HTTPS`
- ❌ `HTTP + 局域网IP` (会被阻止)

## 解决方案

### 方案1：使用 localhost（推荐）

**适用场景**：在运行服务器的同一台电脑上使用

```bash
# 1. 启动服务器
cd /home/puji/Lab2QRCode/web-frontend
python3 serve.py &

# 2. 浏览器访问
打开: http://localhost:5000
```

✅ 这样可以直接使用摄像头！

---

### 方案2：使用 HTTPS（支持远程访问）

**适用场景**：需要从其他设备（手机、平板）访问摄像头

#### 步骤1：生成SSL证书（只需一次）

```bash
cd /home/puji/Lab2QRCode/web-frontend
./generate-cert.sh
```

#### 步骤2：启动HTTPS服务器

```bash
cd /home/puji/Lab2QRCode/web-frontend

# 停止HTTP服务器
pkill -f "serve.py"

# 启动HTTPS服务器
python3 serve-https.py &
```

#### 步骤3：访问并信任证书

**在本机访问**：
1. 打开 https://localhost:5000
2. 浏览器会显示"不安全"警告（因为是自签名证书）
3. 点击 **"高级"** → **"继续访问"**

**从MacBook访问服务器**：
1. 打开 https://192.168.9.100:5000
2. 同样会有安全警告
3. 点击 **"高级"** → **"继续访问"**

**Safari特殊说明**：
Safari可能需要额外步骤：
1. 下载证书：`scp user@192.168.9.100:/home/puji/Lab2QRCode/web-frontend/certs/cert.pem ~/Downloads/`
2. 双击证书文件，添加到钥匙串
3. 在"钥匙串访问"中找到证书，设置为"始终信任"

---

### 方案3：仅生成功能（无需摄像头）

如果只需要生成条码功能，使用HTTP即可：

```bash
cd /home/puji/Lab2QRCode/web-frontend
python3 serve.py &
```

访问: http://localhost:5000 或 http://192.168.9.100:5000

✅ Generate（生成）功能正常工作
❌ Scan（扫描）功能需要HTTPS

---

## 快速测试

### 测试HTTP服务器（生成功能）

```bash
curl http://localhost:5000
# 应该返回HTML
```

### 测试HTTPS服务器（全部功能）

```bash
curl -k https://localhost:5000
# -k 参数跳过证书验证
```

---

## 服务器管理

### 查看运行状态

```bash
ps aux | grep "serve"
```

### 停止服务器

```bash
# 停止HTTP服务器
pkill -f "serve.py"

# 停止HTTPS服务器
pkill -f "serve-https.py"
```

### 查看日志

```bash
tail -f /tmp/lab2qrcode-server.log
```

---

## 访问地址总结

| 服务类型 | 地址 | 摄像头 | 适用场景 |
|---------|------|--------|---------|
| HTTP | http://localhost:5000 | ✅ | 本机开发 |
| HTTP | http://192.168.9.100:5000 | ❌ | 局域网（仅生成） |
| HTTPS | https://localhost:5000 | ✅ | 本机（全功能） |
| HTTPS | https://192.168.9.100:5000 | ✅ | 局域网（全功能） |

---

## 常见问题

### Q1: 为什么HTTPS显示不安全？
A: 因为使用的是自签名证书。这对开发环境是正常的。点击"继续访问"即可。

### Q2: MacBook上Safari无法访问摄像头？
A: Safari对自签名证书更严格。按照上面的"Safari特殊说明"安装证书。

### Q3: 我只想生成二维码，不需要摄像头？
A: 使用HTTP服务器即可：`python3 serve.py`，访问 http://localhost:5000

### Q4: 如何从手机访问？
A: 
1. 启动HTTPS服务器：`python3 serve-https.py`
2. 手机连接同一WiFi
3. 访问：https://192.168.9.100:5000
4. 信任证书警告

---

## 推荐配置

**开发调试**：
```bash
python3 serve.py
# 访问 http://localhost:5000
```

**完整功能（含摄像头）**：
```bash
python3 serve-https.py
# 访问 https://localhost:5000
```
