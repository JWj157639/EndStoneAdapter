# 程序智能重启工具

## 项目概述

Windows程序崩溃自动重启工具是一个轻量级的控制台应用程序，用于监控指定的Windows进程，并在进程崩溃或意外退出时自动重新启动该程序。该工具特别适用于需要高可用性的服务器应用程序和关键业务程序。

## 核心功能特性

### 基础功能
- **自动监控**: 实时监控指定进程的运行状态
- **自动重启**: 检测到进程退出时自动重启程序
- **可配置参数**: 支持自定义重启延迟、检测间隔和最大重启次数

### 日志系统
- **多级日志**: 提供DEBUG、INFO、WARN、ERROR、FATAL五个日志等级
- **彩色输出**: 控制台日志使用不同颜色区分日志等级
- **UTF-8支持**: 完整支持中文等Unicode字符

### 交互功能
- **指令控制**: 支持通过命令行指令控制程序运行
- **MQTT远程控制**: 可选功能，支持通过MQTT协议进行远程控制和状态监控

## 项目结构详解

本项目采用模块化设计，主要包含以下目录结构：

```
Restart/
├── src/                        # 源代码目录
│   ├── main.cpp               # 主程序入口
│   ├── logger.h/cpp           # 日志模块
│   ├── input_module.h/cpp     # 用户输入模块
│   ├── restart_module.h/cpp   # 重启模块
│   ├── process_manager.h/cpp  # 进程管理模块
│   └── mqtt_module.h/cpp      # MQTT模块（可选）
├── docs/                       # 文档目录
│   ├── README.md              # 项目说明
│   ├── BUILD.md               # 编译指南
│   ├── USAGE.md               # 使用说明
│   ├── ARCHITECTURE.md        # 架构说明
│   └── COMMANDS.md            # 命令参考
├── project/                    # 项目依赖
│   └── paho.mqtt.cpp/         # Paho MQTT C++库
├── build.bat                   # Windows编译脚本
└── README.txt                  # 原始说明文件
```

## 快速开始指南

### 编译项目

#### 自动编译（推荐）

使用提供的编译脚本（自动检测MQTT库）：

```batch
build.bat
```

#### 手动编译

**基础版本（无MQTT功能）**：

```batch
g++ -o console_auto_restart.exe src/main.cpp src/logger.cpp src/input_module.cpp src/restart_module.cpp src/process_manager.cpp -std=c++11 -O2 -Wall -lpsapi -lkernel32 -static
```

**完整版本（含MQTT功能）**：

```batch
g++ -o console_auto_restart.exe src/main.cpp src/logger.cpp src/input_module.cpp src/restart_module.cpp src/process_manager.cpp src/mqtt_module.cpp -std=c++17 -O2 -Wall -DPAHO_MQTT_ENABLED -I"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\externals\paho-mqtt-c\paho.mqtt.c-1.3.12\src" -I"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\include" -L"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\build\src" -L"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\build\externals\paho-mqtt-c\src" -lpaho-mqttpp3-static -lpaho-mqtt3a-static -lpsapi -lkernel32 -lws2_32 -lcrypt32 -ladvapi32 -lrpcrt4 -static
```

### 运行程序

#### 基本用法

```batch
console_auto_restart.exe <进程名> <可执行文件路径>
```

#### 使用示例

```batch
console_auto_restart.exe notepad.exe "C:\Windows\System32\notepad.exe"
```

### MQTT功能配置（可选）

如需使用MQTT远程控制功能，需要先编译Paho MQTT库：

```batch
cd project\paho.mqtt.cpp
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_SHARED=FALSE -DPAHO_BUILD_STATIC=TRUE
mingw32-make
```

编译完成后，程序将自动连接到localhost:1883的MQTT服务器，并支持以下远程功能：
- `restart/control/{进程名}` 主题：接收控制指令
- 支持RESTART、STOP、START、STATUS等指令
- 发布状态信息到 `restart/status/{进程名}` 主题

## 系统要求

### 操作系统
- Windows 7 或更高版本

### 编译工具
- MinGW-w64 或 TDM-GCC 编译器（包含g++）
- CMake（编译MQTT功能时需要）
- Visual Studio 2015 或更高版本（可选）

## 许可证

本项目仅供学习和个人使用。

## 联系方式

如有问题或建议，请提交 Issue。