# 备份模块 (Backup Module) 文档

## 概述

备份模块提供了自动化的数据备份功能，支持定时备份和手动备份。该模块采用冷备份策略，在备份前会停止目标进程，备份完成后由外部逻辑控制目标进程的重启。

## JSON 配置说明

备份模块的配置存储在 `mqtt-config.json` 文件的 `modules.backup` 节点中。

### 配置结构

```json
{
  "modules": {
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
      "target_window_title": "d:\\mengli\\bedrock_server_mod.exe",
      "stop_command": "stop",
      "stop_delay_seconds": 5,
      "restart_delay_seconds": 5
    }
  }
}
```

### 配置参数说明

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `enabled` | boolean | false | 是否启用备份模块 |
| `auto_backup` | boolean | false | 是否启用自动备份（注意：当前版本备份模块不提供自动备份监控功能） |
| `cold_backup` | boolean | true | 是否启用冷备份（停止进程后备份） |
| `world_dirs` | array | ["world"] | 需要备份的目录路径列表，支持多个路径，每个路径将被合并到同一个备份文件中 |
| `backup_dir` | string | "backups" | 备份文件保存的目录 |
| `backup_name_format` | string | "%Y-%m-%d_%H-%M-%S" | 备份文件名的时间格式（支持strftime格式） |
| `backup_retention_count` | number | 5 | 保留的备份文件数量（-1表示无限制） |
| `clear_before_backup` | boolean | false | 是否先清理旧备份再进行新备份 |
| `compress_format` | string | "zip" | 压缩格式（zip或7z） |
| `target_process_name` | string | "bedrock_server_mod.exe" | 目标进程名称（第0个参数） |
| `target_window_title` | string | "d:\\mengli\\bedrock_server_mod.exe" | 目标窗口标题（用于定位窗口） |
| `stop_command` | string | "stop" | 向目标进程发送的停止命令 |
| `stop_delay_seconds` | number | 5 | 发送停止命令后等待的秒数 |
| `restart_delay_seconds` | number | 5 | 备份完成后重启前等待的秒数 |

## 项目级别接口 (API)

### 初始化与管理接口

#### `bool BackupModule_Initialize()`
初始化备份模块，从配置文件加载配置。

- **参数**: 无
- **返回值**: 成功返回 true，失败返回 false
- **说明**: 应在程序启动时调用

#### `void BackupModule_Cleanup()`
清理备份模块资源。

- **参数**: 无
- **返回值**: 无

#### `void BackupModule_Unload()`
卸载备份模块并释放所有动态分配的内存。

- **参数**: 无
- **返回值**: 无
- **说明**: 完全卸载备份模块，回收所有堆栈和内存资源，模块进入未加载状态

### 备份操作接口

#### `BackupResult BackupModule_PerformBackup()`
执行一次手动备份操作。

- **参数**: 无
- **返回值**: 返回 BackupResult 枚举值
  - `BACKUP_SUCCESS` (1): 备份成功
  - `BACKUP_FAILED` (0): 备份失败
  - `BACKUP_NOT_INIT` (-1): 模块未初始化
  - `BACKUP_IN_PROGRESS` (-2): 备份已在进行中
  - `BACKUP_INVALID_PARAM` (-3): 参数无效
- **说明**: 执行冷备份流程

### 配置管理接口

#### `bool BackupModule_SetConfig(BackupConfig* new_config)`
设置备份模块配置。

- **参数**: `new_config` - 新的配置结构体指针
- **返回值**: 成功返回 true，失败返回 false

#### `BackupConfig* BackupModule_GetConfig()`
获取当前备份模块配置。

- **参数**: 无
- **返回值**: 配置结构体指针，失败返回 NULL

#### `bool BackupModule_LoadConfig(const std::string& configPath)`
从指定路径加载配置文件。

- **参数**: `configPath` - 配置文件路径
- **返回值**: 成功返回 true，失败返回 false

### 重启模块协调接口

#### `void BackupModule_SetRestartModuleActive(bool active)`
设置重启模块的激活状态。

- **参数**: `active` - 激活状态
- **返回值**: 无
- **说明**: 用于备份期间临时停用重启模块

#### `bool BackupModule_IsRestartModuleActive()`
检查重启模块是否处于激活状态。

- **参数**: 无
- **返回值**: 激活返回 true，否则返回 false

## 内部函数说明

### 核心功能函数

#### `bool BackupModule_CreateZip(const char* source_dirs[], int num_dirs, const char* output_path)`
将指定目录压缩为ZIP文件。

- **参数**: 
  - `source_dirs` - 源目录路径数组
  - `num_dirs` - 目录数量
  - `output_path` - 输出文件路径
- **返回值**: 成功返回 true，失败返回 false

#### `void BackupModule_ClearOldBackups()`
清理旧的备份文件，保留数量由配置决定。

- **参数**: 无
- **返回值**: 无

#### `bool BackupModule_StopTargetProcess()`
停止目标进程，通过窗口句柄发送停止命令。

- **参数**: 无
- **返回值**: 成功返回 true，失败返回 false

## Windows窗口通信机制

### `BackupModule_StopTargetProcess` 函数实现细节

此函数遵循Windows窗口通信机制，使用 `SendInput+SetFocus+AttachThreadInput` 方案，确保与目标窗口的可靠通信：

1. **窗口定位**：优先通过 `target_window_title` 查找窗口，如果找不到再通过 `target_process_name` 查找
2. **线程绑定**：使用 `AttachThreadInput` 将当前线程与目标窗口线程绑定，解决 `SetFocus` 的线程限制
3. **焦点设置**：使用 `SetFocus` 设置输入焦点到目标窗口，严格校验返回值
4. **键盘模拟**：使用 `SendInput` 模拟键盘输入，精确发送停止命令
5. **资源释放**：确保线程解绑和焦点恢复，避免系统输入异常

### 安全措施
- 检查窗口是否可见且启用
- 在完成操作后解绑线程
- 恢复原有焦点窗口
- 提供完整的错误处理机制

## 使用示例

### 按需加载和使用备份模块
```cpp
// 1. 初始化并加载备份模块（按需加载到内存）
if (!BackupModule_Initialize()) {
    // 处理初始化失败
    return -1;
}

// 2. 执行一次手动备份
BackupResult backupResult = BackupModule_PerformBackup();
if (backupResult == BACKUP_SUCCESS) {
    printf("备份成功\n");
} else {
    printf("备份失败，错误码: %d\n", backupResult);
}

// 3. 卸载备份模块（释放内存）
BackupModule_Unload();
```

### 修改配置
```cpp
// 获取当前配置
BackupConfig* config = BackupModule_GetConfig();
if (config) {
    // 修改配置
    config->auto_backup = true;
    strcpy(config->backup_times[0], "03:00");
    config->num_backup_times = 1;
    
    // 应用新配置
    BackupModule_SetConfig(config);
}
```

## 工作流程

1. **按需加载阶段**：
   - 调用 `BackupModule_Initialize()` 初始化备份模块
   - 从 `mqtt-config.json` 加载配置
   - 模块动态加载到内存

2. **备份阶段**：
   - 调用 `BackupModule_PerformBackup()` 执行备份
   - 检查重启模块是否运行，如运行则临时停用
   - 使用 `SendInput+SetFocus+AttachThreadInput` 方案停止目标进程
   - 执行备份操作
   - 备份完成后，目标进程的重启由外部逻辑控制

3. **卸载阶段**：
   - 调用 `BackupModule_Unload()` 卸载备份模块
   - 释放动态分配的内存
   - 关闭所有句柄并回收堆栈

## 备份操作时序图

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

## 备份失败后的恢复流程

### 当前备份失败处理机制
当备份操作失败时，系统会执行以下步骤：

1. **错误记录**：在 `BackupModule_PerformBackup()` 函数中记录错误日志
2. **状态更新**：将备份状态设置为非进行中状态
3. **资源清理**：释放备份操作占用的相关资源
4. **状态上报**：通过外部机制报告失败状态

### 建议的备份失败恢复流程
为了完善当前的备份失败恢复机制，建议实施以下流程：

#### 1. 自动重试机制
- 当备份失败时，系统应自动重试3次，每次重试间隔30秒
- 重试失败后，将错误信息记录到错误日志

#### 2. 临时文件清理
- 备份失败后，清理所有临时文件和未完成的备份文件
- 验证源目录状态，确保没有文件锁定

#### 3. 进程状态恢复
- 确保在备份失败后，目标进程处于正确的运行状态
- 如果目标进程因备份操作停止，需要重新启动

#### 4. 通知机制
- 在控制台输出详细错误信息
- 提供错误日志记录

#### 5. 手动恢复
- 提供手动恢复命令供用户执行
- 在配置文件中记录失败的备份尝试，便于后续分析

## 备份文件格式详细说明

### 备份文件命名格式
备份文件按以下格式命名：
```
backup_YYYY-MM-DD_HH-MM-SS.zip
```
其中：
- `YYYY` - 年份（4位）
- `MM` - 月份（2位）
- `DD` - 日期（2位）
- `HH` - 小时（24小时制，2位）
- `MM` - 分钟（2位）
- `SS` - 秒（2位）

命名格式可在配置文件中通过 `backup_name_format` 参数自定义，支持 `strftime` 格式。

### 备份文件结构
压缩包内部结构：
```
backup_YYYY-MM-DD_HH-MM-SS/
├── world/ (或其他指定的世界目录)
│   ├── level.dat
│   ├── region/
│   └── ...
├── ...
```

### 备份文件压缩格式
- 默认使用 `ZIP` 格式压缩
- 可通过配置文件设置为 `7Z` 格式
- 使用系统 `powershell Compress-Archive` 命令或 `7z` 命令创建压缩文件

### 备份文件存储位置
- 默认存储在 `backups` 目录
- 可通过配置文件中的 `backup_dir` 参数指定
- 支持相对路径和绝对路径

### 备份文件保留策略
- 默认保留最近5个备份文件
- 可通过配置文件中的 `backup_retention_count` 参数设置保留数量
- 设置为-1表示无限保留
- 超出保留数量的旧备份会被自动删除

### 备份文件验证
- 备份成功后会记录文件大小
- 通过 `GetFileSize` 函数验证文件是否存在且大小大于0
- 备份失败时会记录详细的错误信息

## 注意事项

- 冷备份会停止目标进程，确保在合适的时间执行备份
- 备份模块不会自动重启目标进程，需要外部逻辑控制
- 配置中的窗口标题必须与目标进程的实际窗口标题完全匹配
- 压缩格式支持ZIP和7Z，但需要系统中安装相应工具
- **重要**：使用 `SendInput` 模拟键盘输入需要管理员权限，且目标窗口必须可见和启用