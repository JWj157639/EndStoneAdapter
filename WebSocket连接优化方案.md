# WebSocket连接优化方案

## 当前实现分析

### 现有机制

#### 1. 心跳机制
```cpp
// 当前实现：固定间隔心跳
void BotClient::sendHeart(){
    json emptyJson;
    sendMessage(ServerSendEvent::heart,emptyJson);
}

// 在握手成功后设置心跳任务
void BotClient::shakedProcess(){
    heartTask = HuHoBot::getInstance().setHeartTask();
}
```

**特点**：
- 固定时间间隔发送心跳
- 服务器响应确认连接状态
- 简单可靠，但不够灵活

#### 2. 自动重连机制
```cpp
// 当前实现：固定重连次数和延迟
void BotClient::task_reconnect(){
    if(shouldReconnect && reconnectCount < maxReconnectCount){
        reconnectCount++;
        logger->info("正在尝试重新连接,这是第({}/{})次连接", reconnectCount,maxReconnectCount);
        reconnect();
    }
}
```

**特点**：
- 最大重连次数：5次
- 简单的重连计数
- 没有退避策略

#### 3. 连接状态管理
```cpp
// 当前实现：基本状态管理
enum class Status {
    Open, Closing, Closed
};

std::atomic<Status> status;
```

**特点**：
- 三种基本状态
- 原子操作保证线程安全
- 状态转换简单

#### 4. 消息发送
```cpp
// 当前实现：直接发送，有互斥锁保护
void WebSocketClient::SendText(const string& text) {
    if(status.load() != Status::Open)
        throw std::runtime_error("WebSocket is not open.");
    PrivateMembers->Send(WebSocketOpcode::Text, text.begin(), text.end(), sendMutex);
}
```

**特点**：
- 同步发送
- 互斥锁保护
- 无消息队列

## 优化方案

### 1. 智能心跳机制

#### 1.1 动态心跳间隔

**实现思路**：
```cpp
class HeartbeatManager {
private:
    std::chrono::milliseconds baseInterval{30000};  // 基础间隔30秒
    std::chrono::milliseconds minInterval{10000};   // 最小间隔10秒
    std::chrono::milliseconds maxInterval{60000};   // 最大间隔60秒
    std::atomic<int> networkQuality{100};           // 网络质量评分
    
public:
    void updateNetworkQuality(int latency) {
        // 根据延迟调整网络质量
        if(latency < 100) {
            networkQuality = 100;
        } else if(latency < 500) {
            networkQuality = 80;
        } else if(latency < 1000) {
            networkQuality = 60;
        } else {
            networkQuality = 40;
        }
    }
    
    std::chrono::milliseconds getNextInterval() {
        // 根据网络质量动态调整心跳间隔
        double factor = networkQuality.load() / 100.0;
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
            baseInterval * factor
        );
        
        // 限制在最小和最大间隔之间
        return std::max(minInterval, std::min(maxInterval, interval));
    }
};
```

**优势**：
- 网络良好时减少心跳频率，节省资源
- 网络差时增加心跳频率，及时发现问题
- 自适应网络环境变化

#### 1.2 心跳响应时间监控

```cpp
class HeartbeatMonitor {
private:
    std::deque<std::chrono::milliseconds> responseTimes;
    static constexpr size_t MAX_SAMPLES = 10;
    
public:
    void recordResponse(std::chrono::milliseconds responseTime) {
        responseTimes.push_back(responseTime);
        if(responseTimes.size() > MAX_SAMPLES) {
            responseTimes.pop_front();
        }
    }
    
    std::chrono::milliseconds getAverageResponse() {
        if(responseTimes.empty()) return std::chrono::milliseconds{0};
        
        auto sum = std::accumulate(responseTimes.begin(), responseTimes.end(), 
                                   std::chrono::milliseconds{0});
        return sum / responseTimes.size();
    }
    
    bool isConnectionHealthy() {
        auto avg = getAverageResponse();
        return avg < std::chrono::milliseconds{1000};  // 1秒内响应认为健康
    }
};
```

### 2. 智能重连策略

#### 2.1 指数退避重连

```cpp
class SmartReconnectManager {
private:
    struct ReconnectAttempt {
        int attempt;
        std::chrono::system_clock::time_point timestamp;
        std::string reason;
    };
    
    std::vector<ReconnectAttempt> reconnectHistory;
    std::chrono::milliseconds baseDelay{1000};     // 基础延迟1秒
    std::chrono::milliseconds maxDelay{60000};     // 最大延迟60秒
    int maxAttempts = 10;                          // 最大重连次数
    
public:
    std::chrono::milliseconds getNextDelay(int attempt) {
        // 指数退避：delay = base * 2^(attempt-1)
        auto delay = baseDelay * static_cast<int>(std::pow(2, attempt - 1));
        return std::min(delay, maxDelay);
    }
    
    bool shouldReconnect(const std::string& reason) {
        // 根据断开原因决定是否重连
        if(reason.find("timeout") != std::string::npos) {
            return true;  // 超时可以重连
        }
        if(reason.find("refused") != std::string::npos) {
            return false; // 连接被拒绝，不重连
        }
        if(reason.find("shutdown") != std::string::npos) {
            return false; // 服务器主动关闭，不重连
        }
        return true;  // 默认重连
    }
    
    void recordAttempt(int attempt, const std::string& reason) {
        reconnectHistory.push_back({
            attempt,
            std::chrono::system_clock::now(),
            reason
        });
        
        // 只保留最近20次记录
        if(reconnectHistory.size() > 20) {
            reconnectHistory.erase(reconnectHistory.begin());
        }
    }
    
    double getSuccessRate() {
        if(reconnectHistory.empty()) return 0.0;
        
        int successful = std::count_if(reconnectHistory.begin(), reconnectHistory.end(),
            [](const ReconnectAttempt& att) {
                return att.reason.find("success") != std::string::npos;
            });
        
        return static_cast<double>(successful) / reconnectHistory.size();
    }
};
```

#### 2.2 连接质量评估

```cpp
class ConnectionQualityAssessor {
private:
    struct QualityMetrics {
        int totalConnections = 0;
        int successfulConnections = 0;
        std::chrono::milliseconds totalLatency{0};
        int disconnectCount = 0;
        std::chrono::system_clock::time_point lastDisconnect;
    };
    
    QualityMetrics metrics;
    
public:
    void recordConnection(bool success, std::chrono::milliseconds latency) {
        metrics.totalConnections++;
        if(success) {
            metrics.successfulConnections++;
            metrics.totalLatency += latency;
        }
    }
    
    void recordDisconnect() {
        metrics.disconnectCount++;
        metrics.lastDisconnect = std::chrono::system_clock::now();
    }
    
    double getSuccessRate() const {
        if(metrics.totalConnections == 0) return 0.0;
        return static_cast<double>(metrics.successfulConnections) / metrics.totalConnections;
    }
    
    std::chrono::milliseconds getAverageLatency() const {
        if(metrics.successfulConnections == 0) return std::chrono::milliseconds{0};
        return metrics.totalLatency / metrics.successfulConnections;
    }
    
    int getDisconnectCount() const {
        return metrics.disconnectCount;
    }
    
    bool isConnectionStable() const {
        // 成功率>80%且最近5分钟没有断开认为稳定
        auto now = std::chrono::system_clock::now();
        auto timeSinceDisconnect = std::chrono::duration_cast<std::chrono::minutes>(
            now - metrics.lastDisconnect
        );
        
        return getSuccessRate() > 0.8 && timeSinceDisconnect.count() > 5;
    }
};
```

### 3. 增强的连接状态管理

#### 3.1 扩展状态枚举

```cpp
enum class ConnectionState {
    Disconnected,      // 未连接
    Connecting,        // 连接中
    Handshaking,       // 握手中
    Connected,         // 已连接
    Authenticating,    // 认证中
    Authenticated,     // 已认证
    Degraded,          // 降级状态
    Closing,           // 关闭中
    Closed             // 已关闭
};

class ConnectionStateManager {
private:
    std::atomic<ConnectionState> state{ConnectionState::Disconnected};
    std::mutex stateMutex;
    std::unordered_map<ConnectionState, std::chrono::system_clock::time_point> stateHistory;
    
public:
    bool setState(ConnectionState newState) {
        std::lock_guard<std::mutex> lock(stateMutex);
        
        // 记录状态转换
        auto oldState = state.load();
        stateHistory[oldState] = std::chrono::system_clock::now();
        
        // 验证状态转换是否合法
        if(!isValidTransition(oldState, newState)) {
            return false;
        }
        
        state.store(newState);
        return true;
    }
    
    ConnectionState getState() const {
        return state.load();
    }
    
    std::string getStateString() const {
        switch(state.load()) {
            case ConnectionState::Disconnected: return "Disconnected";
            case ConnectionState::Connecting: return "Connecting";
            case ConnectionState::Handshaking: return "Handshaking";
            case ConnectionState::Connected: return "Connected";
            case ConnectionState::Authenticating: return "Authenticating";
            case ConnectionState::Authenticated: return "Authenticated";
            case ConnectionState::Degraded: return "Degraded";
            case ConnectionState::Closing: return "Closing";
            case ConnectionState::Closed: return "Closed";
            default: return "Unknown";
        }
    }
    
    bool isOperational() const {
        auto s = state.load();
        return s == ConnectionState::Connected || 
               s == ConnectionState::Authenticated ||
               s == ConnectionState::Degraded;
    }
    
private:
    bool isValidTransition(ConnectionState from, ConnectionState to) {
        // 定义合法的状态转换
        static const std::unordered_map<ConnectionState, std::vector<ConnectionState>> validTransitions = {
            {ConnectionState::Disconnected, {ConnectionState::Connecting}},
            {ConnectionState::Connecting, {ConnectionState::Handshaking, ConnectionState::Closed}},
            {ConnectionState::Handshaking, {ConnectionState::Connected, ConnectionState::Closed}},
            {ConnectionState::Connected, {ConnectionState::Authenticating, ConnectionState::Degraded, ConnectionState::Closing}},
            {ConnectionState::Authenticating, {ConnectionState::Authenticated, ConnectionState::Degraded, ConnectionState::Closing}},
            {ConnectionState::Authenticated, {ConnectionState::Degraded, ConnectionState::Closing}},
            {ConnectionState::Degraded, {ConnectionState::Authenticated, ConnectionState::Closing}},
            {ConnectionState::Closing, {ConnectionState::Closed}},
            {ConnectionState::Closed, {ConnectionState::Connecting}}
        };
        
        auto it = validTransitions.find(from);
        if(it == validTransitions.end()) return false;
        
        const auto& validStates = it->second;
        return std::find(validStates.begin(), validStates.end(), to) != validStates.end();
    }
};
```

#### 3.2 连接健康检查

```cpp
class ConnectionHealthChecker {
private:
    struct HealthMetrics {
        std::chrono::system_clock::time_point lastActivity;
        std::chrono::system_clock::time_point lastHeartbeat;
        std::chrono::system_clock::time_point lastError;
        int errorCount = 0;
        int consecutiveErrors = 0;
    };
    
    HealthMetrics metrics;
    std::mutex metricsMutex;
    static constexpr int MAX_CONSECUTIVE_ERRORS = 5;
    
public:
    void recordActivity() {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics.lastActivity = std::chrono::system_clock::now();
        metrics.consecutiveErrors = 0;  // 重置连续错误计数
    }
    
    void recordHeartbeat() {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics.lastHeartbeat = std::chrono::system_clock::now();
    }
    
    void recordError() {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics.lastError = std::chrono::system_clock::now();
        metrics.errorCount++;
        metrics.consecutiveErrors++;
    }
    
    HealthStatus checkHealth() const {
        std::lock_guard<std::mutex> lock(metricsMutex);
        auto now = std::chrono::system_clock::now();
        
        // 检查连续错误
        if(metrics.consecutiveErrors >= MAX_CONSECUTIVE_ERRORS) {
            return HealthStatus::Critical;
        }
        
        // 检查心跳超时
        auto timeSinceHeartbeat = std::chrono::duration_cast<std::chrono::seconds>(
            now - metrics.lastHeartbeat
        );
        if(timeSinceHeartbeat.count() > 120) {  // 2分钟无心跳
            return HealthStatus::Unhealthy;
        }
        
        // 检查活动超时
        auto timeSinceActivity = std::chrono::duration_cast<std::chrono::seconds>(
            now - metrics.lastActivity
        );
        if(timeSinceActivity.count() > 300) {  // 5分钟无活动
            return HealthStatus::Degraded;
        }
        
        return HealthStatus::Healthy;
    }
    
    std::string getHealthReport() const {
        std::lock_guard<std::mutex> lock(metricsMutex);
        auto now = std::chrono::system_clock::now();
        
        std::ostringstream oss;
        oss << "Connection Health Report:\n";
        oss << "  Status: " << toString(checkHealth()) << "\n";
        oss << "  Total Errors: " << metrics.errorCount << "\n";
        oss << "  Consecutive Errors: " << metrics.consecutiveErrors << "\n";
        
        auto timeSinceHeartbeat = std::chrono::duration_cast<std::chrono::seconds>(
            now - metrics.lastHeartbeat
        );
        oss << "  Time Since Last Heartbeat: " << timeSinceHeartbeat.count() << "s\n";
        
        auto timeSinceActivity = std::chrono::duration_cast<std::chrono::seconds>(
            now - metrics.lastActivity
        );
        oss << "  Time Since Last Activity: " << timeSinceActivity.count() << "s\n";
        
        return oss.str();
    }
    
private:
    enum class HealthStatus {
        Healthy,
        Degraded,
        Unhealthy,
        Critical
    };
    
    std::string toString(HealthStatus status) const {
        switch(status) {
            case HealthStatus::Healthy: return "Healthy";
            case HealthStatus::Degraded: return "Degraded";
            case HealthStatus::Unhealthy: return "Unhealthy";
            case HealthStatus::Critical: return "Critical";
            default: return "Unknown";
        }
    }
};
```

### 4. 消息队列优化

#### 4.1 优先级消息队列

```cpp
enum class MessagePriority {
    Critical,  // 关键消息（如心跳、认证）
    High,      // 高优先级（如命令响应）
    Normal,    // 普通消息（如聊天）
    Low        // 低优先级（如日志）
};

struct QueuedMessage {
    std::string content;
    MessagePriority priority;
    std::chrono::system_clock::time_point timestamp;
    int retryCount = 0;
    static constexpr int MAX_RETRIES = 3;
    
    QueuedMessage(const std::string& msg, MessagePriority prio)
        : content(msg), priority(prio), timestamp(std::chrono::system_clock::now()) {}
};

class MessageQueue {
private:
    struct MessageComparator {
        bool operator()(const QueuedMessage& a, const QueuedMessage& b) const {
            // 优先级高的先发送
            if(a.priority != b.priority) {
                return static_cast<int>(a.priority) > static_cast<int>(b.priority);
            }
            // 同优先级按时间排序
            return a.timestamp > b.timestamp;
        }
    };
    
    std::priority_queue<QueuedMessage, std::vector<QueuedMessage>, MessageComparator> queue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::atomic<bool> running{true};
    std::thread senderThread;
    WebSocketClient* client;
    
    static constexpr size_t MAX_QUEUE_SIZE = 1000;
    static constexpr std::chrono::milliseconds SEND_INTERVAL{10};
    
public:
    MessageQueue(WebSocketClient* wsClient) : client(wsClient) {
        senderThread = std::thread([this]() { senderLoop(); });
    }
    
    ~MessageQueue() {
        running = false;
        queueCV.notify_all();
        if(senderThread.joinable()) {
            senderThread.join();
        }
    }
    
    bool enqueue(const std::string& message, MessagePriority priority = MessagePriority::Normal) {
        std::lock_guard<std::mutex> lock(queueMutex);
        
        if(queue.size() >= MAX_QUEUE_SIZE) {
            return false;  // 队列已满
        }
        
        queue.emplace(message, priority);
        queueCV.notify_one();
        return true;
    }
    
    size_t getQueueSize() const {
        std::lock_guard<std::mutex> lock(queueMutex);
        return queue.size();
    }
    
private:
    void senderLoop() {
        while(running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            queueCV.wait(lock, [this]() {
                return !queue.empty() || !running;
            });
            
            if(!running) break;
            
            while(!queue.empty() && running) {
                auto message = queue.top();
                queue.pop();
                lock.unlock();
                
                try {
                    if(client->GetStatus() == WebSocketClient::Status::Open) {
                        client->SendText(message.content);
                    } else {
                        // 连接不可用，重新入队
                        if(message.retryCount < QueuedMessage::MAX_RETRIES) {
                            message.retryCount++;
                            std::lock_guard<std::mutex> relock(queueMutex);
                            queue.push(message);
                        }
                    }
                } catch(const std::exception& e) {
                    // 发送失败，重新入队
                    if(message.retryCount < QueuedMessage::MAX_RETRIES) {
                        message.retryCount++;
                        std::lock_guard<std::mutex> relock(queueMutex);
                        queue.push(message);
                    }
                }
                
                std::this_thread::sleep_for(SEND_INTERVAL);
                lock.lock();
            }
        }
    }
};
```

#### 4.2 消息压缩

```cpp
class MessageCompressor {
private:
    bool compressionEnabled = true;
    static constexpr size_t COMPRESSION_THRESHOLD = 1024;  // 1KB以上才压缩
    
public:
    std::string compress(const std::string& message) {
        if(!compressionEnabled || message.size() < COMPRESSION_THRESHOLD) {
            return message;
        }
        
        // 使用zlib压缩
        // 这里简化实现，实际应该使用zlib库
        std::string compressed;
        // ... 压缩逻辑 ...
        return compressed;
    }
    
    std::string decompress(const std::string& compressed) {
        // 使用zlib解压
        std::string decompressed;
        // ... 解压逻辑 ...
        return decompressed;
    }
    
    void setCompressionEnabled(bool enabled) {
        compressionEnabled = enabled;
    }
    
    bool isCompressionEnabled() const {
        return compressionEnabled;
    }
};
```

### 5. 错误处理和恢复

#### 5.1 智能错误分类

```cpp
enum class ErrorCategory {
    Network,      // 网络错误
    Protocol,     // 协议错误
    Authentication, // 认证错误
    RateLimit,    // 频率限制
    Server,       // 服务器错误
    Unknown       // 未知错误
};

enum class ErrorSeverity {
    Low,      // 低严重性，可以忽略
    Medium,   // 中等严重性，需要记录
    High,     // 高严重性，需要处理
    Critical  // 严重，需要立即处理
};

struct ErrorInfo {
    ErrorCategory category;
    ErrorSeverity severity;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    int occurrenceCount = 1;
    
    ErrorInfo(ErrorCategory cat, ErrorSeverity sev, const std::string& msg)
        : category(cat), severity(sev), message(msg), 
          timestamp(std::chrono::system_clock::now()) {}
};

class ErrorHandler {
private:
    std::deque<ErrorInfo> errorHistory;
    std::mutex errorMutex;
    static constexpr size_t MAX_ERROR_HISTORY = 100;
    
public:
    void handleError(const std::string& errorMessage) {
        auto errorInfo = classifyError(errorMessage);
        
        std::lock_guard<std::mutex> lock(errorMutex);
        
        // 检查是否是重复错误
        for(auto& error : errorHistory) {
            if(error.message == errorMessage) {
                error.occurrenceCount++;
                return;
            }
        }
        
        // 添加新错误
        errorHistory.push_back(errorInfo);
        if(errorHistory.size() > MAX_ERROR_HISTORY) {
            errorHistory.pop_front();
        }
        
        // 根据严重性处理
        switch(errorInfo.severity) {
            case ErrorSeverity::Critical:
                handleCriticalError(errorInfo);
                break;
            case ErrorSeverity::High:
                handleHighSeverityError(errorInfo);
                break;
            case ErrorSeverity::Medium:
                handleMediumSeverityError(errorInfo);
                break;
            case ErrorSeverity::Low:
                handleLowSeverityError(errorInfo);
                break;
        }
    }
    
    std::vector<ErrorInfo> getRecentErrors(int count = 10) const {
        std::lock_guard<std::mutex> lock(errorMutex);
        std::vector<ErrorInfo> recent;
        
        auto start = errorHistory.size() > static_cast<size_t>(count) 
                     ? errorHistory.end() - count 
                     : errorHistory.begin();
        
        std::copy(start, errorHistory.end(), std::back_inserter(recent));
        return recent;
    }
    
private:
    ErrorInfo classifyError(const std::string& errorMessage) {
        ErrorCategory category = ErrorCategory::Unknown;
        ErrorSeverity severity = ErrorSeverity::Medium;
        
        // 网络错误
        if(errorMessage.find("timeout") != std::string::npos ||
           errorMessage.find("network") != std::string::npos) {
            category = ErrorCategory::Network;
            severity = ErrorSeverity::Medium;
        }
        // 连接错误
        else if(errorMessage.find("connection") != std::string::npos) {
            category = ErrorCategory::Network;
            severity = ErrorSeverity::High;
        }
        // 认证错误
        else if(errorMessage.find("auth") != std::string::npos ||
                errorMessage.find("token") != std::string::npos) {
            category = ErrorCategory::Authentication;
            severity = ErrorSeverity::Critical;
        }
        // 协议错误
        else if(errorMessage.find("protocol") != std::string::npos ||
                errorMessage.find("frame") != std::string::npos) {
            category = ErrorCategory::Protocol;
            severity = ErrorSeverity::High;
        }
        // 频率限制
        else if(errorMessage.find("rate") != std::string::npos ||
                errorMessage.find("limit") != std::string::npos) {
            category = ErrorCategory::RateLimit;
            severity = ErrorSeverity::Medium;
        }
        
        return ErrorInfo(category, severity, errorMessage);
    }
    
    void handleCriticalError(const ErrorInfo& error) {
        // 关键错误：立即断开连接，停止重连
        logger->error("CRITICAL ERROR: {}", error.message);
        // 触发紧急处理流程
    }
    
    void handleHighSeverityError(const ErrorInfo& error) {
        // 高严重性错误：记录日志，可能需要重连
        logger->error("HIGH SEVERITY ERROR: {}", error.message);
        // 考虑触发重连
    }
    
    void handleMediumSeverityError(const ErrorInfo& error) {
        // 中等严重性：记录日志，监控
        logger->warning("MEDIUM SEVERITY ERROR: {}", error.message);
    }
    
    void handleLowSeverityError(const ErrorInfo& error) {
        // 低严重性：记录日志
        logger->info("LOW SEVERITY ERROR: {}", error.message);
    }
};
```

#### 5.2 熔断机制

```cpp
enum class CircuitState {
    Closed,    // 正常状态
    Open,      // 熔断状态
    HalfOpen   // 半开状态（尝试恢复）
};

class CircuitBreaker {
private:
    struct CircuitConfig {
        int failureThreshold = 5;           // 失败阈值
        std::chrono::seconds timeout{60};    // 熔断超时时间
        int successThreshold = 2;           // 恢复成功阈值
    };
    
    CircuitConfig config;
    std::atomic<CircuitState> state{CircuitState::Closed};
    std::atomic<int> failureCount{0};
    std::atomic<int> successCount{0};
    std::chrono::system_clock::time_point lastStateChange;
    std::mutex stateMutex;
    
public:
    bool allowRequest() {
        auto currentState = state.load();
        
        if(currentState == CircuitState::Closed) {
            return true;
        }
        
        if(currentState == CircuitState::Open) {
            // 检查是否超时，可以尝试恢复
            auto timeSinceChange = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - lastStateChange
            );
            
            if(timeSinceChange >= config.timeout) {
                setState(CircuitState::HalfOpen);
                return true;
            }
            return false;
        }
        
        if(currentState == CircuitState::HalfOpen) {
            return true;
        }
        
        return false;
    }
    
    void recordSuccess() {
        auto currentState = state.load();
        
        if(currentState == CircuitState::HalfOpen) {
            successCount++;
            if(successCount >= config.successThreshold) {
                setState(CircuitState::Closed);
                resetCounters();
            }
        } else if(currentState == CircuitState::Closed) {
            failureCount = 0;
        }
    }
    
    void recordFailure() {
        auto currentState = state.load();
        
        if(currentState == CircuitState::HalfOpen) {
            setState(CircuitState::Open);
            resetCounters();
        } else {
            failureCount++;
            if(failureCount >= config.failureThreshold) {
                setState(CircuitState::Open);
            }
        }
    }
    
    CircuitState getState() const {
        return state.load();
    }
    
    std::string getStateString() const {
        switch(state.load()) {
            case CircuitState::Closed: return "Closed";
            case CircuitState::Open: return "Open";
            case CircuitState::HalfOpen: return "HalfOpen";
            default: return "Unknown";
        }
    }
    
    void reset() {
        setState(CircuitState::Closed);
        resetCounters();
    }
    
private:
    void setState(CircuitState newState) {
        std::lock_guard<std::mutex> lock(stateMutex);
        state.store(newState);
        lastStateChange = std::chrono::system_clock::now();
    }
    
    void resetCounters() {
        failureCount = 0;
        successCount = 0;
    }
};
```

### 6. 性能优化

#### 6.1 连接池

```cpp
class ConnectionPool {
private:
    struct PooledConnection {
        std::unique_ptr<WebSocketClient> client;
        std::chrono::system_clock::time_point lastUsed;
        bool inUse = false;
        int useCount = 0;
    };
    
    std::vector<std::unique_ptr<PooledConnection>> pool;
    std::mutex poolMutex;
    std::string serverUrl;
    size_t maxPoolSize = 5;
    size_t minPoolSize = 2;
    
public:
    ConnectionPool(const std::string& url) : serverUrl(url) {
        initializePool();
    }
    
    WebSocketClient* acquireConnection() {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        // 查找可用连接
        for(auto& conn : pool) {
            if(!conn->inUse && conn->client->GetStatus() == WebSocketClient::Status::Open) {
                conn->inUse = true;
                conn->lastUsed = std::chrono::system_clock::now();
                conn->useCount++;
                return conn->client.get();
            }
        }
        
        // 没有可用连接，创建新连接
        if(pool.size() < maxPoolSize) {
            auto newConn = std::make_unique<PooledConnection>();
            newConn->client = std::make_unique<WebSocketClient>();
            newConn->client->Connect(serverUrl);
            newConn->inUse = true;
            newConn->lastUsed = std::chrono::system_clock::now();
            newConn->useCount = 1;
            
            pool.push_back(std::move(newConn));
            return pool.back()->client.get();
        }
        
        return nullptr;  // 连接池已满
    }
    
    void releaseConnection(WebSocketClient* client) {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        for(auto& conn : pool) {
            if(conn->client.get() == client) {
                conn->inUse = false;
                conn->lastUsed = std::chrono::system_clock::now();
                break;
            }
        }
    }
    
    void cleanupIdleConnections() {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        auto now = std::chrono::system_clock::now();
        auto it = pool.begin();
        
        while(it != pool.end()) {
            auto idleTime = std::chrono::duration_cast<std::chrono::minutes>(
                now - (*it)->lastUsed
            );
            
            // 移除空闲超过30分钟且使用次数少的连接
            if(idleTime.count() > 30 && (*it)->useCount < 10 && 
               pool.size() > minPoolSize && !(*it)->inUse) {
                (*it)->client->Shutdown();
                it = pool.erase(it);
            } else {
                ++it;
            }
        }
    }
    
private:
    void initializePool() {
        for(size_t i = 0; i < minPoolSize; ++i) {
            auto conn = std::make_unique<PooledConnection>();
            conn->client = std::make_unique<WebSocketClient>();
            try {
                conn->client->Connect(serverUrl);
                pool.push_back(std::move(conn));
            } catch(const std::exception& e) {
                logger->error("Failed to initialize connection: {}", e.what());
            }
        }
    }
};
```

#### 6.2 批量操作优化

```cpp
class BatchOperationManager {
private:
    struct BatchConfig {
        size_t maxBatchSize = 100;
        std::chrono::milliseconds maxBatchDelay{100};
    };
    
    BatchConfig config;
    std::vector<std::string> pendingMessages;
    std::mutex batchMutex;
    std::chrono::system_clock::time_point lastBatchTime;
    WebSocketClient* client;
    
public:
    BatchOperationManager(WebSocketClient* wsClient) : client(wsClient) {
        lastBatchTime = std::chrono::system_clock::now();
    }
    
    void addMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(batchMutex);
        
        pendingMessages.push_back(message);
        
        // 检查是否需要发送批次
        if(shouldFlush()) {
            flush();
        }
    }
    
    void flush() {
        std::lock_guard<std::mutex> lock(batchMutex);
        
        if(pendingMessages.empty()) return;
        
        // 构建批量消息
        json batch = json::array();
        for(const auto& msg : pendingMessages) {
            batch.push_back(json::parse(msg));
        }
        
        try {
            if(client->GetStatus() == WebSocketClient::Status::Open) {
                client->SendText(batch.dump());
            }
        } catch(const std::exception& e) {
            logger->error("Batch send failed: {}", e.what());
        }
        
        pendingMessages.clear();
        lastBatchTime = std::chrono::system_clock::now();
    }
    
private:
    bool shouldFlush() const {
        // 达到最大批次大小
        if(pendingMessages.size() >= config.maxBatchSize) {
            return true;
        }
        
        // 达到最大延迟时间
        auto timeSinceLastBatch = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastBatchTime
        );
        
        return timeSinceLastBatch >= config.maxBatchDelay;
    }
};
```

## 实施建议

### 优先级排序

1. **高优先级**（立即实施）：
   - 智能重连策略（指数退避）
   - 增强的连接状态管理
   - 错误分类和处理

2. **中优先级**（短期实施）：
   - 动态心跳机制
   - 消息队列优化
   - 连接健康检查

3. **低优先级**（长期优化）：
   - 连接池
   - 消息压缩
   - 批量操作优化

### 渐进式实施

1. **第一阶段**：保持现有功能，添加监控和日志
2. **第二阶段**：实施高优先级优化
3. **第三阶段**：根据监控数据调整参数
4. **第四阶段**：实施中低优先级优化

### 测试策略

1. **单元测试**：每个优化模块独立测试
2. **集成测试**：测试优化模块之间的交互
3. **压力测试**：模拟高负载场景
4. **网络测试**：模拟各种网络条件

## 监控指标

### 关键指标

1. **连接稳定性**：
   - 连接成功率
   - 平均连接时间
   - 断连频率

2. **性能指标**：
   - 消息延迟
   - 吞吐量
   - 资源使用率

3. **可靠性指标**：
   - 错误率
   - 重连成功率
   - 消息丢失率

### 告警阈值

- 连接成功率 < 95%
- 平均延迟 > 1秒
- 错误率 > 5%
- 连续断连 > 3次

## 总结

通过实施这些优化方案，可以显著提升WebSocket连接的：

1. **稳定性**：智能重连和状态管理
2. **性能**：消息队列和批量操作
3. **可靠性**：错误处理和熔断机制
4. **可维护性**：监控和日志系统

建议根据实际业务需求和资源情况，选择合适的优化方案逐步实施。