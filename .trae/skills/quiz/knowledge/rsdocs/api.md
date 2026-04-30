# API Reference

## 重启模块 (Restart Module) API

### 核心功能函数

#### `bool RestartModule::Initialize(const std::string& processName, const std::string& executable, int restartDelay, int checkInterval, int maxRestarts)`
初始化重启模块。

- **参数**:
  - `processName`: 要监控的进程名称
  - `executable`: 要重启的可执行文件路径
  - `restartDelay`: 重启延迟（毫秒）
  - `checkInterval`: 检测间隔（毫秒）
  - `maxRestarts`: 最大重启次数
- **返回值**: 初始化是否成功

#### `void RestartModule::Process()`
处理重启逻辑，在主循环中调用，检查进程状态并执行重启。

#### `void RestartModule::Cleanup()`
清理重启模块资源。

#### `int RestartModule::GetRestartCount()`
获取当前重启次数。

#### `bool RestartModule::IsMonitoring()`
获取监控状态。

#### `HANDLE RestartModule::GetRestartEvent()`
获取重启事件句柄。

#### `void RestartModule::StopMonitoring()`
停止监控。

#### `void RestartModule::StartMonitoring()`
开始监控。

#### `void RestartModule::PauseMonitoring()`
暂停监控（临时停止，不改变监控状态）。

#### `void RestartModule::ResumeMonitoring()`
恢复监控（从暂停状态恢复）。

#### `void RestartModule::ForceRestart()`
强制重启进程。

## 备份模块 (Backup Module) API

### 核心功能函数

#### `bool BackupModule_Initialize()`
初始化备份模块，从配置文件加载配置。

- **返回值**: 成功返回 true，失败返回 false

#### `void BackupModule_Cleanup()`
清理备份模块资源。

#### `void BackupModule_Unload()`
卸载备份模块并释放所有动态分配的内存。

#### `BackupResult BackupModule_PerformBackup()`
执行一次手动备份操作。

- **返回值**: 返回 BackupResult 枚举值
  - `BACKUP_SUCCESS` (1): 备份成功
  - `BACKUP_FAILED` (0): 备份失败
  - `BACKUP_NOT_INIT` (-1): 模块未初始化
  - `BACKUP_IN_PROGRESS` (-2): 备份已在进行中
  - `BACKUP_INVALID_PARAM` (-3): 参数无效

#### `bool BackupModule_SetConfig(BackupConfig* new_config)`
设置备份模块配置。

- **参数**: `new_config` - 新的配置结构体指针
- **返回值**: 成功返回 true，失败返回 false

#### `BackupConfig* BackupModule_GetConfig()`
获取当前备份模块配置。

- **返回值**: 配置结构体指针，失败返回 NULL

#### `bool BackupModule_LoadConfig(const std::string& configPath)`
从指定路径加载配置文件。

- **参数**: `configPath` - 配置文件路径
- **返回值**: 成功返回 true，失败返回 false

#### `void BackupModule_SetRestartModuleActive(bool active)`
设置重启模块的激活状态。

- **参数**: `active` - 激活状态

#### `bool BackupModule_IsRestartModuleActive()`
检查重启模块是否处于激活状态。

- **返回值**: 激活返回 true，否则返回 false

## MQTT业务逻辑模块 (MQTT Business Logic) API

### 核心功能函数

#### `bool MQTTBusinessLogic::Initialize()`
初始化MQTT业务逻辑模块。

- **返回值**: 成功返回 true，失败返回 false

#### `void MQTTBusinessLogic::Cleanup()`
清理MQTT业务逻辑模块资源。

#### `void MQTTBusinessLogic::SetBusinessCallback(BusinessCallback callback)`
设置业务回调函数。

- **参数**: `callback` - 业务回调函数

#### `void MQTTBusinessLogic::InternalMessageCallback(const char* topic, const char* payload, int payload_len)`
内部消息回调函数。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载
  - `payload_len`: 负载长度

#### `void MQTTBusinessLogic::ProcessMessageByTopic(const std::string& topic, const std::string& payload)`
根据主题处理消息。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载

#### `void MQTTBusinessLogic::ProcessControlCommand(const std::string& topic, const std::string& payload)`
处理控制命令。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载

#### `void MQTTBusinessLogic::ProcessStatusMessage(const std::string& topic, const std::string& payload)`
处理状态消息。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载

#### `void MQTTBusinessLogic::ProcessBackupData(const std::string& topic, const std::string& payload)`
处理备份数据。

- **参数**: 
  - `topic`: 主题
  - `payload`: 负载

#### `void MQTTBusinessLogic::ProcessBackupCommand()`
处理备份命令。

#### `void MQTTBusinessLogic::InitializeBackupSystem()`
初始化备份系统。

#### `std::string MQTTBusinessLogic::GetBrokerAddress()`
获取MQTT代理地址。

- **返回值**: 代理地址字符串

#### `int MQTTBusinessLogic::GetBrokerPort()`
获取MQTT代理端口。

- **返回值**: 代理端口整数

#### `std::string MQTTBusinessLogic::GetClientId()`
获取MQTT客户端ID。

- **返回值**: 客户端ID字符串

#### `std::string MQTTBusinessLogic::GetUsername()`
获取MQTT用户名。

- **返回值**: 用户名字符串

#### `std::string MQTTBusinessLogic::GetPassword()`
获取MQTT密码。

- **返回值**: 密码字符串

#### `int MQTTBusinessLogic::GetQos()`
获取MQTT QoS等级。

- **返回值**: QoS等级整数

#### `void MQTTBusinessLogic::ProcessRestartCommand(const std::string& processName)`
处理重启命令。

- **参数**: `processName` - 进程名称

#### `void MQTTBusinessLogic::ProcessStopCommand(const std::string& processName)`
处理停止命令。

- **参数**: `processName` - 进程名称

#### `void MQTTBusinessLogic::ProcessStartCommand(const std::string& processName)`
处理开始命令。

- **参数**: `processName` - 进程名称

#### `void MQTTBusinessLogic::ProcessStatusCommand(const std::string& processName)`
处理状态命令。

- **参数**: `processName` - 进程名称

#### `void MQTTBusinessLogic::PublishStatus(const std::string& processName, const std::string& status)`
发布状态消息。

- **参数**: 
  - `processName`: 进程名称
  - `status`: 状态信息

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

#### `std::string MQTTBusinessLogic::GetTopicWithProcessName(const std::string& topicType, const std::string& processName)`
获取带进程名称的主题。

- **参数**: 
  - `topicType`: 主题类型
  - `processName`: 进程名称
- **返回值**: 主题字符串

## 进程管理模块 (Process Manager) API

### 核心功能函数

#### `static DWORD ProcessManager::FindProcessByName(const std::string& processName)`
根据进程名查找进程ID。

- **参数**: `processName` - 进程名称
- **返回值**: 进程ID，未找到返回0

#### `static bool ProcessManager::StartProcess(const std::string& executable)`
启动进程。

- **参数**: `executable` - 可执行文件路径
- **返回值**: 是否启动成功

#### `static bool ProcessManager::IsProcessRunning(DWORD processId)`
检查进程是否仍在运行。

- **参数**: `processId` - 进程ID
- **返回值**: 是否正在运行

#### `static bool ProcessManager::IsProcessRunning(HANDLE hProcess)`
检查进程是否仍在运行（通过句柄）。

- **参数**: `hProcess` - 进程句柄
- **返回值**: 是否正在运行

## 日志模块 (Logger) API

### 核心功能函数

#### `static void Logger::InitLogger(LogLevel level)`
初始化日志系统。

- **参数**: `level` - 日志等级

#### `static void Logger::Cleanup()`
清理日志系统资源。

#### `static void Logger::SetLogLevel(LogLevel level)`
设置日志等级。

- **参数**: `level` - 日志等级

#### `static LogLevel Logger::GetLogLevel()`
获取当前日志等级。

- **返回值**: 当前日志等级

#### `static void Logger::Log(LogLevel level, const char* format, ...)`
记录日志。

- **参数**: 
  - `level`: 日志等级
  - `format`: 格式字符串
  - `...`: 参数列表

## 输入模块 (Input Module) API

### 核心功能函数

#### `bool InputModule::Initialize()`
初始化输入模块。

- **返回值**: 成功返回 true，失败返回 false

#### `void InputModule::Cleanup()`
清理输入模块资源。

#### `void InputModule::ProcessInput()`
处理用户输入。

#### `bool InputModule::ShouldExit()`
检查是否应该退出。

- **返回值**: 应该退出返回 true，否则返回 false

#### `HANDLE InputModule::GetExitEvent()`
获取退出事件句柄。

- **返回值**: 退出事件句柄

#### `HANDLE InputModule::GetInputEvent()`
获取输入事件句柄。

- **返回值**: 输入事件句柄

## 函数调用时序图

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

### 冷备份执行时序图

```
BackupModule_PerformBackup()
    ↓
检查重启模块状态 (BackupModule_IsRestartModuleActive())
    ↓
停止目标进程 (BackupModule_StopTargetProcess())
    ↓
创建临时目录
    ↓
复制源目录到临时目录 (CopyDirectory)
    ↓
创建压缩文件 (BackupModule_CreateZip())
    ↓
清理旧备份 (BackupModule_ClearOldBackups())
    ↓
返回备份结果 (BackupResult)
```

### MQTT消息处理时序图

```
MQTT消息到达
    ↓
MQTTModule::MessageCallback
    ↓
MQTTBusinessLogic::InternalMessageCallback
    ↓
MQTTBusinessLogic::ProcessMessageByTopic
    ↓
根据主题类型分发:
    ├─> 控制命令 -> ProcessControlCommand
    │   ├─> RESTART -> ProcessRestartCommand
    │   ├─> STOP -> ProcessStopCommand
    │   ├─> START -> ProcessStartCommand
    │   ├─> STATUS -> ProcessStatusCommand
    │   └─> BACKUP_NOW -> ProcessBackupCommand
    │
    ├─> 状态消息 -> ProcessStatusMessage
    │
    └─> 备份数据 -> ProcessBackupData
        └─> 验证帧 -> 解析帧 -> 执行备份
```

### 窗口控制时序图 (BackupModule_StopTargetProcess)

```
BackupModule_StopTargetProcess()
    ↓
按窗口标题查找目标窗口 (FindWindowA)
    ↓
如果按标题未找到，按进程名查找窗口
    ↓
获取目标窗口线程ID和当前程序线程ID
    ↓
如果目标窗口与当前程序不在同一线程，则绑定线程 (AttachThreadInput)
    ↓
设置输入焦点到目标窗口 (SetFocus)
    ↓
准备键盘输入事件 (INPUT数组)
    ↓
执行键盘输入模拟 (SendInput)
    ↓
恢复原焦点窗口
    ↓
解绑线程 (AttachThreadInput)
    ↓
等待进程响应停止命令 (Sleep)
```