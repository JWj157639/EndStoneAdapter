#pragma once

#include "websocketfiles/src/ws_endpoint.h"
#include "websocketfiles/src/ws_packet.h"
#include "endstone/logger.h"
#include "endstone/plugin.h"
#include "endstone/scheduler/scheduler.h"
#include "endstone/scheduler/task.h"
#include "endstone/server.h"
#include "config.h"
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#if defined(_WIN32)
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#if defined(_WIN32)
typedef SOCKET socket_t;
#else
typedef int socket_t;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#define closesocket(s) ::close(s)
#endif

/**
 * WebSocket 连接管理器
 * 
 * 负责管理 WebSocket 连接状态，提供原子操作防止重复连接，
 * 实现梯度延迟重连策略，支持断线数据补全和超时检测
 */
class WsConnectionManager : public WebSocketEndpoint {
public:
    /**
     * 连接状态枚举
     */
    enum class ConnectionStatus {
        SUBSCRIBED,    // 已订阅，连接正常
        CHANNEL_ERROR, // 通道错误
        TIMED_OUT,     // 超时
        CLOSED         // 已关闭
    };

    /**
     * 构造函数
     * @param logger EndStone 日志记录器，用于记录连接状态和错误信息
     * @param plugin EndStone 插件实例，用于获取调度器和调度任务
     * @param url WebSocket 服务器地址，默认使用编译时宏 HUHOBOT_SERVER_URL
     */
    WsConnectionManager(endstone::Logger* logger, endstone::Plugin* plugin, 
                     const std::string& url = HUHOBOT_SERVER_URL);
    
    /**
     * 析构函数
     * 清理资源，关闭连接，停止所有任务
     */
    ~WsConnectionManager();

    /**
     * 初始化连接
     * 使用原子操作防止重复连接，如果已有连接正在进行则跳过
     */
    void initConnect();
    
    /**
     * 关闭连接
     * 停止接收线程，取消所有任务，清理资源
     */
    void closeConnect();
    
    /**
     * 自动重连
     * 根据重连次数使用梯度延迟策略：
     * - 前3次重连：延迟 1 秒
     * - 后续重连：延迟 3 秒
     * 超过最大重连次数后显示错误提示
     */
    void autoReconnect();

    /**
     * 注册状态变化回调
     * @param callback 状态变化时的回调函数，接收状态和错误消息
     */
    void OnStatusChange(std::function<void(ConnectionStatus, std::string)> callback);
    
    /**
     * 注册消息接收回调
     * @param callback 接收到消息时的回调函数，接收消息内容
     */
    void OnMessageReceived(std::function<void(std::string)> callback);

    /**
     * 发送文本消息
     * @param text 要发送的文本消息
     * @throws std::runtime_error 如果未连接则抛出异常
     */
    void SendText(const std::string& text);

private:
    static const int RECONNECT_FAST_DELAY = 1000;  // 快速重连延迟（毫秒）
    static const int RECONNECT_SLOW_DELAY = 3000;  // 慢速重连延迟（毫秒）
    static const int TIMEOUT_SECONDS = 30;        // 超时检测时间（秒）

    std::atomic<bool> isConnecting;  // 原子标志，防止重复连接

    endstone::Logger* logger;        // 日志记录器
    endstone::Scheduler* scheduler;  // 任务调度器
    int reconnectCount;              // 当前重连次数
    int maxReconnectTimes;           // 最大重连次数
    std::string lastMessageId;       // 最后一条消息的 ID
    ConnectionStatus currentStatus;  // 当前连接状态

    std::chrono::system_clock::time_point lastActivityTime;  // 最后活动时间
    std::shared_ptr<endstone::Task> reconnectTask;          // 重连任务
    std::shared_ptr<endstone::Task> timeoutCheckTask;       // 超时检测任务

    std::function<void(ConnectionStatus, std::string)> statusChangeCallback;    // 状态变化回调
    std::function<void(std::string)> messageReceivedCallback;                  // 消息接收回调

    /**
     * 处理状态变化
     * @param status 新的连接状态
     * @param errorMsg 错误消息（如果有）
     */
    void handleStatusChange(ConnectionStatus status, const std::string& errorMsg = "");
    
    /**
     * 同步丢失的消息
     * 在断线重连后，同步断线期间的消息
     */
    void syncLoseMessage();
    
    /**
     * 显示连接提示
     * 提示用户连接状态
     */
    void showConnectTip();
    
    /**
     * 显示错误提示
     * @param errorMsg 错误消息
     */
    void showErrorTip(const std::string& errorMsg);

    /**
     * 用户自定义的数据包处理（重写自 WebSocketEndpoint）
     * @param packet WebSocket 数据包
     * @param frame_payload 帧负载
     * @return 处理结果
     */
    int32_t user_defined_process(WebSocketPacket& packet, ByteBuffer& frame_payload) override;
    
    /**
     * 启动超时检测
     * 每 30 秒检查一次是否有活动，如果没有则触发超时
     */
    void startTimeoutCheck();
    
    /**
     * 停止超时检测
     * 取消超时检测任务
     */
    void stopTimeoutCheck();
    
    /**
     * 生成消息 ID
     * @return 唯一的消息 ID
     */
    std::string generateMessageId();
    
    /**
     * 生成 MD5 哈希
     * @param input 输入字符串
     * @return MD5 哈希值
     */
    std::string generateMd5(const std::string& input);

    /**
     * 网络接收循环
     * 在独立线程中持续接收网络数据
     */
    void networkRecvLoop();
    
    /**
     * 执行 WebSocket 握手
     * @param hostname 主机名
     * @param port 端口号
     * @param path WebSocket 路径
     * @return 握手是否成功
     */
    bool performHandshake(const std::string& hostname, int port, const std::string& path);
    
    /**
     * 网络发送
     * @param data 要发送的数据
     * @param size 数据大小
     */
    void networkSend(const char* data, int64_t size);
    
    /**
     * 连接到主机
     * @param hostname 主机名
     * @param port 端口号
     * @return 套接字描述符
     */
    socket_t hostnameConnect(const std::string& hostname, int port);

    socket_t wsSocket;            // WebSocket 套接字
    std::thread recvLoopThread;    // 接收循环线程
    std::mutex sendMutex;          // 发送互斥锁
    std::string serverUrl;         // 服务器 URL
};