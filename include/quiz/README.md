# WsConnectionManager 使用说明

## 概述

WsConnectionManager 是一个基于 Supabase Realtime 状态管理理念的 WebSocket 连接管理器，提供原子操作防止重复重连、统一状态处理入口、梯度延迟重连策略和断线数据补全机制。

## 核心特性

- ✅ **原子状态机**：使用 `std::atomic<bool>` 防止重复重连
- ✅ **统一状态处理**：所有状态变化通过 `handleStatusChange()` 统一处理
- ✅ **梯度延迟重连**：前3次快速重连（1秒），后续放慢（3秒）
- ✅ **断线数据补全**：重连后自动同步断线期间丢失的数据
- ✅ **异步调度**：使用 EndStone Scheduler 实现异步延迟，不阻塞线程
- ✅ **超时检测**：30秒无活动自动判定为超时断连

## 连接状态

```cpp
enum class ConnectionStatus {
    SUBSCRIBED,      // 连接正常 / 订阅成功
    CHANNEL_ERROR,   // 连接出错（如网络异常、服务端错误）
    TIMED_OUT,       // 连接超时（无响应，判定为断连）
    CLOSED           // 连接主动关闭（用户操作或服务端主动断开）
};
```

## 基本使用

### 1. 初始化连接管理器

```cpp
#include "quiz/WsConnectionManager.h"

class MyBot {
private:
    WsConnectionManager* wsManager;
    endstone::Logger* logger;
    endstone::Scheduler* scheduler;

public:
    MyBot(endstone::Logger* logger, endstone::Scheduler* scheduler) {
        this->logger = logger;
        this->scheduler = scheduler;
        wsManager = new WsConnectionManager(logger, scheduler);

        // 注册状态变化回调
        wsManager->OnStatusChange([this](WsConnectionManager::ConnectionStatus status, std::string msg) {
            this->onStatusChanged(status, msg);
        });

        // 注册消息接收回调
        wsManager->OnMessageReceived([this](std::string msg) {
            this->onMessageReceived(msg);
        });
    }

    void connect(const std::string& url) {
        wsManager->initConnect();
    }

    void onStatusChanged(WsConnectionManager::ConnectionStatus status, std::string msg) {
        switch (status) {
            case WsConnectionManager::ConnectionStatus::SUBSCRIBED:
                logger->info("连接已建立");
                break;
            case WsConnectionManager::ConnectionStatus::CHANNEL_ERROR:
                logger->error("连接错误: {}", msg);
                break;
            case WsConnectionManager::ConnectionStatus::TIMED_OUT:
                logger->warning("连接超时");
                break;
            case WsConnectionManager::ConnectionStatus::CLOSED:
                logger->info("连接已关闭");
                break;
        }
    }

    void onMessageReceived(std::string msg) {
        logger->info("收到消息: {}", msg);
    }

    void sendMessage(const std::string& msg) {
        wsManager->SendText(msg);
    }

    void disconnect() {
        wsManager->closeConnect();
    }
};
```

### 2. 配置参数

在 `config.json` 中添加：

```json
{
  "maxReconnectTimes": 10
}
```

- `maxReconnectTimes`：最大重连次数，默认为 10

## API 参考

### 公有方法

#### `initConnect()`
初始化 WebSocket 连接。

```cpp
void initConnect();
```

**说明**：
- 使用原子变量 `isConnecting` 防止重复连接
- 如果已有连接正在进行，会跳过本次请求
- 连接失败会自动触发重连机制

#### `closeConnect()`
主动关闭连接。

```cpp
void closeConnect();
```

**说明**：
- 停止所有异步任务
- 关闭 TCP socket
- 停止接收线程

#### `autoReconnect()`
自动重连（通常由内部自动调用）。

```cpp
void autoReconnect();
```

**说明**：
- 检查重连次数是否超限
- 使用梯度延迟策略
- 通过 Scheduler 异步执行，不阻塞线程

#### `OnStatusChange(callback)`
注册状态变化回调。

```cpp
void OnStatusChange(std::function<void(ConnectionStatus, std::string)> callback);
```

**参数**：
- `callback`：回调函数，接收状态和错误信息

**示例**：
```cpp
wsManager->OnStatusChange([](ConnectionStatus status, std::string msg) {
    // 处理状态变化
});
```

#### `OnMessageReceived(callback)`
注册消息接收回调。

```cpp
void OnMessageReceived(std::function<void(std::string)> callback);
```

**参数**：
- `callback`：回调函数，接收消息内容

**示例**：
```cpp
wsManager->OnMessageReceived([](std::string msg) {
    // 处理接收到的消息
});
```

#### `SendText(text)`
发送文本消息。

```cpp
void SendText(const std::string& text);
```

**参数**：
- `text`：要发送的文本内容

**异常**：
- 如果连接状态不是 `SUBSCRIBED`，会抛出异常

## 重连策略

### 梯度延迟

```cpp
int delay = (reconnectCount < 3) ? 1000 : 3000;
```

- 前 3 次重连：延迟 1 秒
- 第 4 次及以后：延迟 3 秒

### 重连次数限制

```cpp
if (reconnectCount >= maxReconnectTimes) {
    showErrorTip("无法恢复连接，请刷新页面");
    return;
}
```

- 达到最大重连次数后停止重连
- 默认最大次数：10（可配置）

## 超时检测

### 检测机制

```cpp
void startTimeoutCheck() {
    timeoutCheckTask = scheduler->runTask([this]() {
        if (currentStatus == ConnectionStatus::SUBSCRIBED) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastActivityTime
            ).count();

            if (elapsed >= TIMEOUT_SECONDS) {
                handleStatusChange(ConnectionStatus::TIMED_OUT, "连接超时");
            }
        }
    }, std::chrono::seconds(5));
}
```

- 每 5 秒检查一次
- 超过 30 秒无活动判定为超时
- 自动触发重连机制

## 断线数据补全

### 消息 ID 机制

```cpp
void syncLoseMessage() {
    std::string newMessageId = generateMessageId();

    if (!lastMessageId.empty()) {
        // 触发回调，通知应用层同步数据
        if (statusChangeCallback) {
            statusChangeCallback(ConnectionStatus::SUBSCRIBED,
                               "需要同步数据: " + lastMessageId);
        }
    }

    lastMessageId = newMessageId;
}
```

- 使用 `tools::generate_pack_id()` 生成唯一消息 ID
- 重连成功后触发回调，通知应用层同步断线期间的数据

## 线程安全

### 原子变量

```cpp
std::atomic<bool> isConnecting;
```

- 使用 CAS 操作防止重复连接
- 线程安全的状态检查

### 互斥锁

```cpp
std::mutex sendMutex;
```

- 保护发送操作
- 防止多线程同时发送数据

## 架构设计

### 分层架构

```
┌─────────────────────────────────┐
│     应用层（对外接口）           │
│  - initConnect()                │
│  - closeConnect()               │
│  - autoReconnect()               │
└─────────────────────────────────┘
              ↓
┌─────────────────────────────────┐
│   状态管理层（核心逻辑）          │
│  - handleStatusChange()         │
│  - autoReconnect()               │
│  - syncLoseMessage()             │
└─────────────────────────────────┘
              ↓
┌─────────────────────────────────┐
│   协议层（websocketfiles）       │
│  - 继承 WebSocketEndpoint       │
│  - 重写 user_defined_process()   │
│  - 使用 WebSocketPacket         │
└─────────────────────────────────┘
              ↓
┌─────────────────────────────────┐
│   网络层（复用 WebSocketClient） │
│  - hostname_connect()           │
│  - TCP socket 管理              │
│  - send/recv 原生 socket        │
└─────────────────────────────────┘
```

## 注意事项

### 1. 线程安全
- 所有回调函数可能在不同的线程中调用
- 确保回调函数中的操作是线程安全的
- 使用适当的同步机制保护共享资源

### 2. 资源管理
- 确保在析构时调用 `closeConnect()`
- 正确处理异常情况
- 避免内存泄漏

### 3. 错误处理
- 捕获所有可能的异常
- 记录详细的错误日志
- 提供友好的错误提示

### 4. 性能优化
- 避免在回调中执行耗时操作
- 使用异步方式处理消息
- 合理设置缓冲区大小

## 常见问题

### Q: 如何禁用自动重连？
A: 将 `maxReconnectTimes` 设置为 0。

### Q: 如何修改超时时间？
A: 修改 `WsConnectionManager.h` 中的 `TIMEOUT_SECONDS` 常量。

### Q: 如何自定义重连延迟？
A: 修改 `WsConnectionManager.h` 中的 `RECONNECT_FAST_DELAY` 和 `RECONNECT_SLOW_DELAY` 常量。

### Q: 如何处理重连后的数据同步？
A: 在状态变化回调中检查错误信息，如果包含 "需要同步数据"，则执行相应的数据同步逻辑。

## 技术支持

如有问题，请参考：
- [计划.md](./计划.md) - 详细的实现计划
- [WebSocket连接优化方案.md](../../WebSocket连接优化方案.md) - WebSocket 优化方案
- [项目架构文档.md](../../项目架构文档.md) - 项目整体架构