# 命令参考

## 命令行参数总览

### 基本语法格式

```batch
console_auto_restart.exe <进程名> <可执行文件路径> [选项]
```

### 参数详细说明

#### 必需参数

| 参数 | 类型 | 说明 | 示例 |
|------|------|------|------|
| 进程名 | 字符串 | 要监控的进程名称 | `notepad.exe` |
| 可执行文件路径 | 字符串 | 要重启的可执行文件完整路径 | `"C:\Windows\System32\notepad.exe"` |

#### 可选参数

| 选项 | 参数类型 | 说明 | 默认值 | 示例 |
|------|----------|------|--------|------|
| -d | 整数（毫秒） | 重启延迟时间 | 3000 | `-d 5000` |
| -c | 整数（毫秒） | 检测间隔时间 | 5000 | `-c 3000` |
| -m | 整数 | 最大重启次数 | 10 | `-m 5` |
| -l | 字符串 | 日志等级 | info | `-l debug` |
| -h | - | 显示帮助信息 | - | `-h` |

## 日志等级配置

| 等级 | 说明 | 使用场景 |
|------|------|----------|
| debug | 调试信息 | 开发和调试时使用 |
| info | 一般信息 | 正常运行信息 |
| warn | 警告信息 | 可能的问题 |
| error | 错误信息 | 操作失败 |
| fatal | 致命错误 | 程序无法继续运行 |

## 程序运行时指令

程序运行时，可以在控制台输入以下指令进行控制：

### exit 指令

**语法**: `exit`

**说明**: 退出程序

**示例**:
```
> exit
正在退出程序...
```

### help 指令

**语法**: `help`

**说明**: 显示帮助信息

**示例**:
```
> help

可用指令:
  exit             退出程序
  help             显示此帮助信息
  status           显示当前状态
```

### status 指令

**语法**: `status`

**说明**: 显示当前状态

**示例**:
```
> status

当前状态:
  程序运行中
  监控状态: 运行中
  重启次数: 0
```

## MQTT远程控制指令（可选功能）

如果程序编译时定义了 `PAHO_MQTT_ENABLED` 宏并包含了MQTT功能，可以通过MQTT服务器发送远程指令。

### MQTT主题配置

| 主题 | 说明 | 方向 |
|------|------|------|
| restart/control/{进程名} | 接收远程控制指令 | 订阅 |
| restart/status/{进程名} | 发布状态信息 | 发布 |

### MQTT控制指令详细说明

#### RESTART 指令

**消息内容**: `RESTART`

**说明**: 手动触发重启操作

**示例**:
```json
{
  "topic": "restart/control/notepad.exe",
  "payload": "RESTART"
}
```

#### STOP 指令

**消息内容**: `STOP`

**说明**: 停止监控进程

**示例**:
```json
{
  "topic": "restart/control/myapp.exe",
  "payload": "STOP"
}
```

#### START 指令

**消息内容**: `START`

**说明**: 开始监控进程（如果已停止）

**示例**:
```json
{
  "topic": "restart/control/server.exe",
  "payload": "START"
}
```

#### STATUS 指令

**消息内容**: `STATUS`

**说明**: 请求当前状态信息

**示例**:
```json
{
  "topic": "restart/control/app.exe",
  "payload": "STATUS"
}
```

## 指令使用规范

### 控制台指令基本规则

1. **大小写不敏感**: 指令不区分大小写
   ```
   > EXIT     # 有效
   > exit     # 有效
   > Exit     # 有效
   ```

2. **回车确认**: 每个指令后需要按回车键执行

3. **空格忽略**: 指令前后的空格会被忽略
   ```
   >   exit   # 等同于 exit
   ```

4. **未知指令**: 输入未知指令会显示错误提示
   ```
   > unknown
   未知指令: unknown
   输入 'help' 查看可用指令
   ```

### MQTT指令格式规范

1. **有效载荷**: 纯文本格式，不区分大小写
2. **主题格式**: `restart/control/{进程名}`
3. **QoS**: 建议使用 QoS 0 或 1

### 特殊字符处理规则

- **空格**: 指令中的空格会被作为分隔符
- **Tab**: 与空格等效
- **退格键**: 可以删除已输入的字符

## 使用示例合集

### 完整命令行示例

```batch
# 监控记事本，使用默认参数
console_auto_restart.exe notepad.exe "C:\Windows\System32\notepad.exe"

# 自定义重启延迟和检测间隔
console_auto_restart.exe myapp.exe "C:\Program Files\MyApp\myapp.exe" -d 5000 -c 3000

# 设置最大重启次数和日志等级
console_auto_restart.exe myapp.exe "C:\MyApp\myapp.exe" -m 20 -l debug

# 组合所有参数
console_auto_restart.exe server.exe "D:\Server\server.exe" -d 10000 -c 5000 -m 100 -l info
```

### 运行时指令使用示例

```
========================================
程序智能重启工具已启动
========================================
可用指令:
  exit  - 退出程序
  help  - 显示帮助
  status- 显示状态
========================================

> help

可用指令:
  exit             退出程序
  help             显示此帮助信息
  status           显示当前状态

> status

当前状态:
  程序运行中
  监控状态: 运行中
  重启次数: 0

> exit
正在退出程序...
```

### MQTT远程控制编程示例

```python
# Python示例：发送MQTT指令
import paho.mqtt.client as mqtt

client = mqtt.Client()
client.connect("localhost", 1883, 60)

# 手动触发重启
client.publish("restart/control/notepad.exe", "RESTART")

# 停止监控
client.publish("restart/control/notepad.exe", "STOP")

client.disconnect()
```

## 参数限制和建议

### 重启延迟 (-d)
- 最小值: 0 毫秒
- 最大值: 无限制
- 默认值: 3000 毫秒
- 推荐范围: 1000-10000 毫秒

### 检测间隔 (-c)
- 最小值: 100 毫秒
- 最大值: 无限制
- 默认值: 5000 毫秒
- 推荐范围: 1000-60000 毫秒

### 最大重启次数 (-m)
- 最小值: 0
- 最大值: 无限制
- 默认值: 10
- 推荐范围: 5-100

## 错误处理和输出

### 参数错误处理

```
console_auto_restart.exe
```
**输出**:
```
Windows程序崩溃自动重启工具 (控制台版)
使用方法: console_auto_restart.exe <进程名> <可执行文件路径> [选项]
...
```

### 文件路径错误处理

```
console_auto_restart.exe myapp.exe "C:\Invalid\path.exe"
```
**输出**:
```
[时间] [ERROR] 启动进程失败: C:\Invalid\path.exe, 错误代码: 2
```

### 参数值错误处理

```
console_auto_restart.exe myapp.exe "C:\MyApp\myapp.exe" -c 50
```
**输出**:
```
[时间] [WARN] 检测间隔不能小于100ms，使用默认值 5000 ms
```

### MQTT连接错误处理

```
[时间] [WARN] MQTT模块连接失败，将继续运行（无MQTT控制功能）
```

## 最佳实践指南

### 参数选择建议

#### 服务器应用配置
- 检测间隔: 10-30 秒
- 重启延迟: 5-10 秒
- 最大重启次数: 50-100

#### 桌面应用配置
- 检测间隔: 3-5 秒
- 重启延迟: 2-5 秒
- 最大重启次数: 10-20

#### 开发测试配置
- 检测间隔: 1-2 秒
- 重启延迟: 1 秒
- 最大重启次数: 5-10
- 日志等级: debug

#### 生产环境配置
- 检测间隔: 5-10 秒
- 重启延迟: 3-5 秒
- 最大重启次数: 20-50
- 日志等级: info

### MQTT远程控制最佳实践

- **QoS等级**: 使用QoS 1确保消息可靠传递
- **主题命名**: 遵循 `restart/control/{进程名}` 格式
- **指令频率**: 避免过于频繁的远程指令
- **监控状态**: 定期检查MQTT连接状态

### 性能优化建议

1. 避免过短的检测间隔（小于1秒）
2. 根据应用重要性调整重启次数
3. 生产环境使用 info 日志等级
4. 开发环境使用 debug 日志等级