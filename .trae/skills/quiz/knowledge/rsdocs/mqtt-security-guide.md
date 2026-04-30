# 生产级MQTT安全配置指南

## 统一主题结构

所有MQTT通信现在使用统一的主题结构：

- **订阅**: `qtop/2026/{processName}` - 接收所有控制命令
- **响应**: `qtop/2026/response/{processName}` - 发送命令响应
- **状态**: `qtop/2026/status/{processName}` - 发布状态信息
- **健康检查**: `qtop/2026/health/{processName}` - 发布健康信息
- **CMD响应**: `qtop/2026/cmd/response/{processName}` - CMD命令响应

## 支持的命令

通过统一主题发送以下命令：

- `RESTART` - 重启目标进程
- `STOP` - 停止监控
- `START` - 开始监控
- `STATUS` - 查询状态
- `BACKUP_NOW` - 立即执行备份
- `CMD <command>` - 执行CMD命令 (例如: `CMD dir`)

## 生产级密钥管理

### 1. 密钥来源优先级

系统按以下优先级获取PSK:

1. **环境变量**: `MQTT_PSK`
2. **密钥文件**: `mqtt_psk.key`
3. **自动生成**: 生成新的32字节安全密钥

### 2. 安全密钥生成

使用 `KeyManager` 类生成安全密钥:

```cpp
// 生成32字节安全密钥
std::string psk = KeyManager::GenerateSecureKey(32);

// 保存到安全位置
KeyManager::SaveKeyToFile(psk, "secure/mqtt_psk.key");
```

### 3. 环境变量设置 (推荐)

在生产环境中推荐通过环境变量设置密钥:

```bash
# Windows
set MQTT_PSK=your_secure_key_here

# Linux/Unix
export MQTT_PSK=your_secure_key_here
```

### 4. 密钥文件安全

- 将密钥文件保存在安全位置
- 设置适当的文件权限
- 定期轮换密钥

### 5. 设备特定密钥

系统支持为不同设备生成特定密钥:

```cpp
std::string deviceSpecificKey = KeyManager::GenerateDeviceKey(baseKey, deviceId);
```

## 客户端重构建议

### 1. 简化的订阅逻辑

客户端只需订阅一个统一主题:

```python
# Python MQTT客户端示例
client.subscribe(f"qtop/2026/{process_name}")
```

### 2. 命令发送

通过统一主题发送所有命令:

```python
# 发送重启命令
client.publish(f"qtop/2026/{process_name}", "RESTART")

# 执行CMD命令
client.publish(f"qtop/2026/{process_name}", "CMD dir")
```

### 3. 响应处理

系统会根据命令类型在相应的响应主题上发布结果:

- 控制命令响应: `qtop/2026/response/{processName}`
- CMD命令响应: `qtop/2026/cmd/response/{processName}`
- 状态更新: `qtop/2026/status/{processName}`

## 安全最佳实践

1. **使用强密钥**: 密钥长度至少32字节，包含随机字符
2. **定期轮换**: 定期更换密钥
3. **限制权限**: 仅授权需要访问的设备
4. **安全传输**: 使用TLS/SSL加密MQTT连接
5. **监控审计**: 记录所有命令执行日志

## 配置示例

```json
{
  "modules": {
    "mqtt": {
      "enabled": true,
      "brokerAddress": "localhost",
      "brokerPort": 1883,
      "topics": {
        "unified": {
          "subscribe": "qtop/2026/{processName}",
          "publish": "qtop/2026/response/{processName}"
        },
        "status": {
          "publish": "qtop/2026/status/{processName}"
        }
      }
    }
  }
}
```

## 故障排除

- **命令无响应**: 检查是否订阅了正确的统一主题
- **鉴权失败**: 验证密钥是否正确设置
- **连接问题**: 确认MQTT服务器地址和端口