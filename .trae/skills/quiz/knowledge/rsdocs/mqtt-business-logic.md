# MQTT Business Logic 模块文档

## 概述

MQTT Business Logic 模块是整个系统中负责处理MQTT消息业务逻辑的核心组件。该模块接收来自MQTT模块的消息，根据主题和负载内容执行相应的业务操作，包括远程控制、状态管理、备份操作等。

## 主要功能

### 1. 消息路由
- 根据MQTT主题路由消息到相应的处理函数
- 支持多种消息类型，包括控制命令、状态更新、备份数据等

### 2. 远程控制
- 处理远程重启、停止、开始、状态查询等指令
- 支持备份操作的远程触发

### 3. 状态管理
- 通过MQTT发布系统状态信息
- 监控系统健康状况

### 4. 配置管理
- 从配置文件加载MQTT业务逻辑相关配置
- 支持动态配置更新

## 配置说明

MQTT Business Logic 模块的配置存储在 `mqtt-config.json` 文件中。

### 配置结构

```json
{
  "version": 1,
  "modules": {
    "mqtt": {
      "enabled": true,
      "brokerAddress": "localhost",
      "brokerPort": 1883,
      "clientId": "restart_tool_client",
      "username": "",
      "password": "",
      "qos": 0,
      "reconnectInterval": 5000,
      "connectionTimeout": 10000,
      "topics": {
        "control": {
          "subscribe": "restart/control/{processName}",
          "publish": "restart/control/response/{processName}"
        },
        "status": {
          "publish": "restart/status/{processName}",
          "subscribe": "restart/status/{processName}"
        },
        "health": {
          "publish": "restart/health/{processName}",
          "interval": 30000
        }
      },
      "commands": {
        "restart": {
          "enabled": true,
          "permission": "admin"
        },
        "stop": {
          "enabled": true,
          "permission": "admin"
        },
        "start": {
          "enabled": true,
          "permission": "admin"
        },
        "status": {
          "enabled": true,
          "permission": "user"
        }
      }
    },
    "backup": {
      "enabled": false,
      "auto_backup": false,
      "cold_backup": true,
      "world_dirs": ["world"],
      "backup_dir": "backups",
      "backup_name_format": "%Y-%m-%d_%H-%M-%S",
      "backup_retention_count": 5,
      "clear_before_backup": false,
      "compress_format": "zip",
      "target_process_name": "bedrock_server_mod.exe",
      "target_window_title": "",
      "stop_command": "stop",
      "stop_delay_seconds": 5,
      "restart_delay_seconds": 5
    }
  },
  "businessLogic": {
    "enableRemoteControl": true,
    "enableStatusReporting": true,
    "enableHealthCheck": true,
    "maxRetries": 3,
    "retryDelay": 2000
  }
}
```

### 配置参数说明

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `enableRemoteControl` | boolean | true | 是否启用远程控制功能 |
| `enableStatusReporting` | boolean | true | 是否启用状态报告功能 |
| `enableHealthCheck` | boolean | true | 是否启用健康检查功能 |
| `maxRetries` | number | 3 | 最大重试次数 |
| `retryDelay` | number | 2000 | 重试延迟时间（毫秒） |

## API接口

### 初始化和管理接口

#### `bool MQTTBusinessLogic::Initialize()`
初始化MQTT业务逻辑模块。

- **返回值**: 成功返回 true，失败返回 false
- **说明**: 应在程序启动时调用

#### `void MQTTBusinessLogic::Cleanup()`
清理MQTT业务逻辑模块资源。

- **参数**: 无
- **返回值**: 无

#### `void MQTTBusinessLogic::SetBusinessCallback(BusinessCallback callback)`
设置业务回调函数。

- **参数**: `callback` - 业务回调函数
- **返回值**: 无

### 消息处理接口

#### `void MQTTBusinessLogic::InternalMessageCallback(const char* topic, const char* payload, int payload_len)`
内部消息回调函数。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载
  - `payload_len`: 负载长度
- **返回值**: 无

#### `void MQTTBusinessLogic::ProcessMessageByTopic(const std::string& topic, const std::string& payload)`
根据主题处理消息。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载
- **返回值**: 无

### 命令处理接口

#### `void MQTTBusinessLogic::ProcessControlCommand(const std::string& topic, const std::string& payload)`
处理控制命令。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载
- **返回值**: 无

#### `void MQTTBusinessLogic::ProcessStatusMessage(const std::string& topic, const std::string& payload)`
处理状态消息。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载
- **返回值**: 无

#### `void MQTTBusinessLogic::ProcessBackupData(const std::string& topic, const std::string& payload)`
处理备份数据。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载
- **返回值**: 无

#### `void MQTTBusinessLogic::ProcessBackupCommand()`
处理备份命令。

- **参数**: 无
- **返回值**: 无

### 操作执行接口

#### `void MQTTBusinessLogic::ProcessRestartCommand(const std::string& processName)`
处理重启命令。

- **参数**: `processName` - 进程名称
- **返回值**: 无

#### `void MQTTBusinessLogic::ProcessStopCommand(const std::string& processName)`
处理停止命令。

- **参数**: `processName` - 进程名称
- **返回值**: 无

#### `void MQTTBusinessLogic::ProcessStartCommand(const std::string& processName)`
处理开始命令。

- **参数**: `processName` - 进程名称
- **返回值**: 无

#### `void MQTTBusinessLogic::ProcessStatusCommand(const std::string& processName)`
处理状态命令。

- **参数**: `processName` - 进程名称
- **返回值**: 无

### 状态发布接口

#### `void MQTTBusinessLogic::PublishStatus(const std::string& processName, const std::string& status)`
发布状态消息。

- **参数**: 
  - `processName`: 进程名称
  - `status`: 状态信息
- **返回值**: 无

### 配置管理接口

#### `bool MQTTBusinessLogic::LoadConfig(const std::string& configPath)`
加载配置文件。

- **参数**: `configPath` - 配置文件路径
- **返回值**: 成功返回 true，失败返回 false

#### `bool MQTTBusinessLogic::CreateDefaultConfig(const std::string& configPath)`
创建默认配置文件。

- **参数**: `configPath` - 配置文件路径
- **返回值**: 成功返回 true，失败返回 false

#### `void MQTTBusinessLogic::SetProcessName(const std::string& processName)`
设置进程名称。

- **参数**: `processName` - 进程名称
- **返回值**: 无

#### `std::string MQTTBusinessLogic::GetTopicWithProcessName(const std::string& topicType, const std::string& processName)`
获取带进程名称的主题。

- **参数**: 
  - `topicType`: 主题类型
  - `processName`: 进程名称
- **返回值**: 主题字符串

## 工作流程

### 1. 初始化阶段
- 调用 `MQTTBusinessLogic::Initialize()` 初始化模块
- 加载配置文件
- 初始化备份系统

### 2. 消息处理阶段
- 通过 `InternalMessageCallback` 接收来自MQTT模块的消息
- 根据主题类型分发到相应的处理函数
- 执行业务逻辑操作

### 3. 命令执行阶段
- 解析命令内容
- 执行相应的控制操作
- 发布操作结果

## 数据流图

### MQTT业务逻辑数据流

```
MQTT消息接收
  │
  ▼
MQTTModule::MessageCallback
  │
  ▼
MQTTBusinessLogic::InternalMessageCallback
  │
  ▼
MQTTBusinessLogic::ProcessMessageByTopic
  │
  ├─> 控制命令 (restart/control/*)
  │   └─> MQTTBusinessLogic::ProcessControlCommand
  │       ├─> RESTART -> ProcessRestartCommand
  │       ├─> STOP -> ProcessStopCommand
  │       ├─> START -> ProcessStartCommand
  │       ├─> STATUS -> ProcessStatusCommand
  │       └─> BACKUP_NOW -> ProcessBackupCommand
  │
  ├─> 状态消息 (restart/status/*)
  │   └─> MQTTBusinessLogic::ProcessStatusMessage
  │
  └─> 备份数据 (backup/*)
      └─> MQTTBusinessLogic::ProcessBackupData
          └─> 验证帧 -> 解析帧 -> 执行备份
```

### 远程备份命令执行时序图

```
MQTT客户端 -> MQTT服务器 -> MQTTModule -> MQTTBusinessLogic::ProcessBackupCommand()
    ↓
MQTTBusinessLogic::ProcessBackupCommand()
    ↓
暂停监控 (RestartModule::PauseMonitoring())
    ↓
备份模块初始化 (BackupModule_Initialize())
    ↓
执行备份 (BackupModule_PerformBackup())
    ↓
恢复监控 (RestartModule::ResumeMonitoring())
    ↓
清理备份模块 (BackupModule_Cleanup())
    ↓
卸载备份模块 (BackupModule_Unload())
    ↓
发布状态 (PublishStatus)
```

## 配置主题格式

### 主题模板
- `{processName}` - 将被替换为实际的进程名称

### 默认主题配置
- 控制订阅: `restart/control/{processName}`
- 控制发布: `restart/control/response/{processName}`
- 状态发布: `restart/status/{processName}`
- 状态订阅: `restart/status/{processName}`
- 健康检查发布: `restart/health/{processName}`

## 错误处理

### 业务逻辑错误处理
- 对于无效的命令，记录警告日志并发送错误响应
- 对于失败的操作，发布失败状态到MQTT主题
- 对于配置错误，记录错误日志并尝试使用默认值

### 重试机制
- 支持配置最大重试次数 (maxRetries)
- 支持配置重试延迟时间 (retryDelay)

## 注意事项

- 需要确保MQTT模块已正确初始化才能接收消息
- 业务逻辑模块依赖于RestartModule和BackupModule
- 远程控制功能需要在配置中启用
- 某些命令可能需要特定权限才能执行