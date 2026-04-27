# AbyssWalker C2

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-red)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-black)
![License](https://img.shields.io/badge/license-Apache%202.0-blue)

**深渊行者 | 企业级红队C2框架**

*Walking through the abyss, leaving no traces behind.*

</div>

---

## ⚠️ 项目声明

**AbyssWalker C2** 是基于 [AdaptixC2](https://github.com/Adaptix-Framework/AdaptixC2) 进行深度魔改的增强版本。

### 与原版的主要区别

| 特性 | AdaptixC2 | AbyssWalker |
|------|-----------|-------------|
| UI框架 | Qt Widgets | Qt Quick/QML (现代化) |
| 加密算法 | RC4 | AES-256-GCM |
| 流量伪装 | 基础 | Malleable Profile系统 |
| 免杀能力 | 基础API Hashing | 完整免杀框架 |
| 跨平台Agent | 部分支持 | 完整三平台支持 |
| 持久化 | 无 | 多平台持久化模块 |

---

## 🔥 核心特性

### 现代化UI (Qt Quick/QML)
- 🎨 专业Light风格界面设计
- 📊 实时仪表盘监控
- 🗂️ Agent表格/拓扑图双视图
- 💻 内置终端、文件浏览器、进程浏览器
- 🔐 凭据管理、目标管理、隧道管理

### 免杀框架

#### Windows Agent (C++)
| 模块 | 技术 | 描述 |
|------|------|------|
| **StringCrypt** | 编译时XOR加密 | 所有敏感字符串加密 |
| **AmsiBypass** | 内存Patch | 绕过AMSI扫描 |
| **EtwPatch** | 函数Hook | 禁用ETW遥测 |
| **Unhooker** | KnownDlls + Syscall | 移除EDR Hook |
| **StackSpoof** | 调用栈伪造 | 绕过调用栈分析 |
| **Injector** | Early Bird/APC | 多种注入技术 |
| **AntiDebug** | PEB/硬件断点 | 10+反调试技术 |
| **AntiSandbox** | 环境/虚拟机检测 | 沙箱逃逸 |
| **SleepMask** | 内存加密 | 睡眠时加密代码段 |
| **Persistence** | 注册表/服务/WMI/计划任务 | 多种持久化方式 |

#### Cross-Platform Agent (Go)
| 平台 | 免杀 | 持久化 |
|------|------|--------|
| **Windows** | AMSI/ETW Bypass, 反调试, 反沙箱 | 注册表, 服务, WMI, 计划任务 |
| **Linux** | 反调试, 反沙箱 | Cron, Systemd, LD_PRELOAD |
| **macOS** | 反调试, 反沙箱 | LaunchAgent, LaunchDaemon |

### 流量隐藏
- 🔐 **AES-256-GCM** 强加密替代RC4
- 🎭 **Malleable Profile** 流量伪装系统
- 🌐 **预置模板**: OneDrive, Teams, Google, Slack, Outlook
- 🔄 **域前置**: CloudFront, Azure CDN支持
- 🔌 **WebSocket Listener** 全双工通信

---

## 📁 项目结构

```
AbyssWalker/
├── Client/                     # Qt Quick/QML 客户端
│   ├── qml/
│   │   ├── components/         # UI组件
│   │   ├── pages/              # 页面
│   │   ├── dialogs/            # 对话框
│   │   └── styles/             # 主题
│   └── cpp/                    # C++后端
│
├── Server/                     # Go服务端
│   ├── core/                   # 核心模块
│   └── extenders/
│       ├── abyss_agent/        # Windows Agent (C++)
│       ├── walker_agent/       # 跨平台Agent (Go)
│       └── listeners/          # 监听器
│
└── profiles/                   # Malleable Profile模板
```

---

## 🛠️ 构建

### 前置要求

| 组件 | 版本 |
|------|------|
| Qt | 6.5+ |
| Go | 1.21+ |
| MinGW-w64 | 最新版 |
| CMake | 3.28+ |

### 构建客户端

```bash
cd Client
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt6
cmake --build . --config Release
```

### 构建服务端

```bash
cd Server
go build -o abysswalker main.go
```

### 构建Windows Agent

```bash
cd Server/extenders/abyss_agent/src
make all
```

### 构建跨平台Agent

```bash
cd Server/extenders/walker_agent/src

# Windows
GOOS=windows GOARCH=amd64 go build -ldflags="-s -w" -o walker_windows.exe

# Linux
GOOS=linux GOARCH=amd64 go build -ldflags="-s -w" -o walker_linux

# macOS
GOOS=darwin GOARCH=amd64 go build -ldflags="-s -w" -o walker_darwin
```

---

## ⚙️ 配置

### Malleable Profile 示例

```yaml
name: "onedrive"
description: "伪装为OneDrive同步流量"

http_get:
  uri:
    - "/personal/{user_id}/_layouts/15/onedrive.aspx"
  verb: "GET"
  client:
    headers:
      Accept: "application/json"
      User-Agent: "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
    metadata:
      encoding: [base64]
      location: "header"
      name: "Cookie"

jitter:
  min: 30
  max: 60
  percent: 20
```

### 域前置配置

```yaml
domain_fronting:
  enabled: true
  cdn_provider: "cloudfront"
  front_domain: "a0.awsstatic.com"
  real_domain: "your-domain.com"
```

---

## 🎯 快速开始

### 启动服务端

```bash
./abysswalker -profile config.yaml
```

### 连接客户端

1. 启动 `AbyssWalkerClient`
2. 输入服务器地址和密码
3. 开始管理你的Agent

### 生成Agent

1. 点击 "Build Agent"
2. 选择协议、平台、架构
3. 勾选免杀选项
4. 生成并部署

---

## 🔒 安全声明

```
⚠️ 法律警告 ⚠️

本工具仅用于授权的安全测试和红队演练。
未经授权使用可能违反当地法律法规。

使用本工具即表示您同意：
1. 仅在获得书面授权的目标上使用
2. 遵守所有适用的法律法规
3. 承担使用本工具的全部法律责任
```

---

## 📜 开源协议

本项目采用 **Apache License 2.0** 开源协议。

### 基于以下开源项目

- [AdaptixC2](https://github.com/Adaptix-Framework/AdaptixC2) - 原始C2框架

### 致谢

感谢 AdaptixC2 团队的优秀工作，本项目在其基础上进行了以下增强：

- UI现代化改造 (Qt Widgets → Qt Quick/QML)
- 加密算法升级 (RC4 → AES-256-GCM)
- 完整免杀框架实现
- Malleable Profile流量伪装系统
- 跨平台持久化模块
- WebSocket监听器

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request。

---

## 📊 功能对比

| 功能 | AbyssWalker | Cobalt Strike | Sliver | Havoc |
|------|-------------|---------------|--------|-------|
| 跨平台Agent | ✅ | ❌ | ✅ | ✅ |
| 现代化UI | ✅ | ❌ | ❌ | ✅ |
| Malleable Profile | ✅ | ✅ | ❌ | ❌ |
| 域前置 | ✅ | ✅ | ✅ | ✅ |
| AMSI Bypass | ✅ | ✅ | ❌ | ✅ |
| Sleep Mask | ✅ | ✅ | ❌ | ✅ |
| 开源 | ✅ | ❌ | ✅ | ✅ |

---

<div align="center">

**⚡ AbyssWalker C2 - 深渊行者，无痕渗透 ⚡**

*Based on AdaptixC2 | Enhanced for Red Team Operations*

</div>
