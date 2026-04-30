# 编译指南

本文档详细介绍了如何编译 Windows 程序崩溃自动重启工具。项目支持多种编译方式，包括基础版本（无MQTT功能）和完整版本（含MQTT远程控制功能）。

## 环境要求

### 必需工具

编译本项目需要以下工具：

- **GCC 编译器**: MinGW-w64 或 TDM-GCC
- **Windows SDK**: 用于 psapi.lib 等库文件

### 可选工具（用于MQTT功能）

如需编译包含MQTT远程控制功能的完整版本，需要以下工具：

- **Paho MQTT C++ 库**: 用于MQTT远程控制功能
  - 需要先编译 `paho.mqtt.cpp` 库

### 推荐工具

以下工具可提升开发体验：

- **Visual Studio**: 用于更好的调试体验
- **CMake**: 跨平台构建支持

## 编译方法总览

本项目支持以下三种编译方法：

1. [使用批处理脚本](#方法一使用批处理脚本推荐)（推荐）
2. [手动编译](#方法二手动编译)
3. [使用 CMake](#方法三使用-cmake)

## 方法一：使用批处理脚本（推荐）

项目提供了 `build.bat` 编译脚本，自动检测MQTT库并选择合适的编译方式。

### 执行编译

```batch
build.bat
```

### 脚本功能说明

该脚本会自动执行以下操作：
- 检测Paho MQTT库是否存在
- 如果存在，编译包含MQTT功能的完整版本
- 如果不存在，编译基础版本（无MQTT功能）

## 方法二：手动编译

手动编译分为基础版本和完整版本两种：

### 基础版本（无MQTT功能）

编译不包含MQTT功能的基础版本：

```batch
cd src
g++ -o ../console_auto_restart.exe main.cpp logger.cpp input_module.cpp restart_module.cpp process_manager.cpp -std=c++11 -O2 -Wall -lpsapi -lkernel32 -static
```

### 完整版本（含MQTT功能）

编译包含MQTT功能的完整版本需要分两步：

#### 1. 编译Paho MQTT库

首先确保已编译Paho MQTT库：

```batch
cd C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\build
cmake .. -G "MinGW Makefiles" -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_SHARED=FALSE -DPAHO_BUILD_STATIC=TRUE
mingw32-make
```

#### 2. 编译项目

然后编译项目（启用MQTT功能）：

```batch
cd src
g++ -o ../console_auto_restart.exe main.cpp logger.cpp input_module.cpp restart_module.cpp process_manager.cpp mqtt_module.cpp -std=c++17 -O2 -Wall -DPAHO_MQTT_ENABLED -I"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\externals\paho-mqtt-c\paho.mqtt.c-1.3.12\src" -I"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\include" -L"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\build\src" -L"C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp\build\externals\paho-mqtt-c\src" -lpaho-mqttpp3-static -lpaho-mqtt3a-static -lpsapi -lkernel32 -lws2_32 -lcrypt32 -ladvapi32 -lrpcrt4 -static
```

### 其他编译器支持

#### 使用 Clang

```batch
cd src
clang++ -o ../console_auto_restart.exe main.cpp logger.cpp input_module.cpp restart_module.cpp process_manager.cpp -std=c++11 -O2 -Wall -lpsapi -lkernel32 -static
```

#### 使用 Visual Studio

使用Visual Studio编译的步骤：

1. 打开 Visual Studio
2. 创建新的空项目
3. 将 `src` 目录下的所有 `.cpp` 和 `.h` 文件添加到项目
4. 在项目属性中添加链接库：`psapi.lib`
5. 选择 Release 配置
6. 点击"生成解决方案"

## 方法三：使用 CMake

本项目支持使用CMake进行跨平台构建。

### CMakeLists.txt 配置

创建 `CMakeLists.txt` 文件：

```cmake
cmake_minimum_required(VERSION 3.10)
project(AutoRestart)

set(CMAKE_CXX_STANDARD 11)

# 添加源文件
set(SOURCES 
    src/main.cpp
    src/logger.cpp
    src/input_module.cpp
    src/restart_module.cpp
    src/process_manager.cpp
)

# 检查MQTT库
find_library(PAHO_MQTT_PP_LIB paho-mqttpp3-static PATHS "C:/Users/Lenovo/Desktop/1/Restart/project/paho.mqtt.cpp/build/src" NO_DEFAULT_PATH)
find_library(PAHO_MQTT_C_LIB paho-mqtt3a-static PATHS "C:/Users/Lenovo/Desktop/1/Restart/project/paho.mqtt.cpp/build/externals/paho-mqtt-c/src" NO_DEFAULT_PATH)
find_path(PAHO_C_INCLUDE_DIR MQTTAsync.h PATHS "C:/Users/Lenovo/Desktop/1/Restart/project/paho.mqtt.cpp/externals/paho-mqtt-c/paho.mqtt.c-1.3.12/src" NO_DEFAULT_PATH)
find_path(PAHO_INCLUDE_DIR mqtt/async_client.h PATHS "C:/Users/Lenovo/Desktop/1/Restart/project/paho.mqtt.cpp/include" NO_DEFAULT_PATH)

if(PAHO_MQTT_PP_LIB AND PAHO_MQTT_C_LIB AND PAHO_C_INCLUDE_DIR AND PAHO_INCLUDE_DIR)
    message(STATUS "Paho MQTT库已找到，启用MQTT功能")
    list(APPEND SOURCES src/mqtt_module.cpp)
    target_compile_definitions(console_auto_restart PRIVATE PAHO_MQTT_ENABLED)
    set_property(TARGET console_auto_restart PROPERTY CXX_STANDARD 17)
    target_include_directories(console_auto_restart PRIVATE ${PAHO_C_INCLUDE_DIR} ${PAHO_INCLUDE_DIR})
    target_link_libraries(console_auto_restart ${PAHO_MQTT_PP_LIB} ${PAHO_MQTT_C_LIB} ws2_32 crypt32 advapi32 rpcrt4)
else()
    message(STATUS "Paho MQTT库未找到，编译基础版本")
endif()

# 创建可执行文件
add_executable(console_auto_restart ${SOURCES})

# 链接库
target_link_libraries(console_auto_restart psapi kernel32)
```

### CMake编译步骤

编译项目：

```batch
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 编译选项详解

### 调试版本

编译包含调试信息的版本：

```batch
g++ -o console_auto_restart.exe src/main.cpp src/logger.cpp src/input_module.cpp src/restart_module.cpp src/process_manager.cpp -g -O0 -std=c++11 -lpsapi -lkernel32 -static
```

### 优化版本

编译经过优化的版本：

```batch
g++ -o console_auto_restart.exe src/main.cpp src/logger.cpp src/input_module.cpp src/restart_module.cpp src/process_manager.cpp -O2 -std=c++11 -lpsapi -lkernel32 -static
```

### 静态链接

静态链接运行时库：

```batch
g++ -o console_auto_restart.exe src/main.cpp src/logger.cpp src/input_module.cpp src/restart_module.cpp src/process_manager.cpp -static -std=c++11 -lpsapi -lkernel32
```

## MQTT功能编译说明

### 编译MQTT库

如需使用MQTT远程控制功能，需要先编译Paho MQTT C++库：

```batch
cd C:\Users\Lenovo\Desktop\1\Restart\project\paho.mqtt.cpp
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_SHARED=FALSE -DPAHO_BUILD_STATIC=TRUE -DCMAKE_POLICY_VERSION_MINIMUM=3.5
mingw32-make
```

### MQTT功能用途

启用MQTT功能后，程序支持以下特性：

- 远程控制重启操作
- 状态监控和上报
- 接收远程指令

## 常见问题及解决方案

### 错误：找不到 psapi.lib

**问题描述**：链接时找不到 `psapi.lib` 库文件

**解决方案**：确保已安装 Windows SDK 或在链接选项中指定正确的库路径。

### 错误：undefined reference to `CreateToolhelp32Snapshot`

**问题描述**：编译时出现 `CreateToolhelp32Snapshot` 未定义的错误

**解决方案**：需要添加 `tlhelp32.h` 头文件，该项目已包含。

### 错误：中文字符乱码

**问题描述**：编译后的程序中文字符显示为乱码

**解决方案**：确保源文件使用 UTF-8 编码保存，编译时添加 `-finput-charset=UTF-8` 选项。

### 错误：无法找到MQTT头文件或库

**问题描述**：编译MQTT版本时找不到相关头文件或库文件

**解决方案**：确保Paho MQTT库已正确编译，或者使用基础版本编译。

## 验证编译结果

编译成功后，运行以下命令验证程序是否正常：

```batch
console_auto_restart.exe -h
```

程序应显示帮助信息，确认编译成功。