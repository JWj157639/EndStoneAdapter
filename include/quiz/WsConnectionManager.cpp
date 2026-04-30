#include "WsConnectionManager.h"
#include "ConfigManager.h"
#include "tools.h"
#include <random>
#include <regex>
#include <sstream>
#include <iomanip>

WsConnectionManager::WsConnectionManager(endstone::Logger* logger, endstone::Scheduler* scheduler)
    : WebSocketEndpoint(),
      logger(logger),
      scheduler(scheduler),
      reconnectCount(0),
      maxReconnectTimes(ConfigManager::Get().GetMaxReconnectTimes()),
      currentStatus(ConnectionStatus::CLOSED),
      isConnecting(false),
      wsSocket(INVALID_SOCKET) {
    lastActivityTime = std::chrono::system_clock::now();
#if defined(_WIN32)
    WSADATA data;
    int WsResult = WSAStartup(MAKEWORD(2, 2), &data);
    if (WsResult != 0) {
        throw std::runtime_error("Can't start winsock.");
    }
#endif
}

WsConnectionManager::~WsConnectionManager() {
    closeConnect();
#if defined(_WIN32)
    WSACleanup();
#endif
}

void WsConnectionManager::initConnect() {
    bool expected = false;
    if (!isConnecting.compare_exchange_strong(expected, true)) {
        logger->info("已有连接正在进行，跳过本次连接请求");
        return;
    }

    try {
        std::regex pattern(R"(ws:\/\/([^:\/]+):?(\d+)?(\/[\S]*)?)");
        std::smatch matches;
        std::regex_search(serverUrl, matches, pattern);
        if (matches.size() == 0) {
            throw std::runtime_error("Unable to parse websocket uri.");
        }

        std::string hostname = matches[1].str();
        int port = matches[2].matched ? std::stoi(matches[2].str()) : 80;
        std::string path = matches[3].matched ? matches[3].str() : "/";

        logger->info("正在连接服务器: {}:{}", hostname, port);

        wsSocket = hostnameConnect(hostname, port);
        if (wsSocket == INVALID_SOCKET) {
            throw std::runtime_error("Unable to connect to " + hostname);
        }

        if (!performHandshake(hostname, port, path)) {
            throw std::runtime_error("Handshake failed");
        }

        logger->info("握手成功");
        handleStatusChange(ConnectionStatus::SUBSCRIBED);

        recvLoopThread = std::thread([this]() { networkRecvLoop(); });

    } catch (const std::exception& e) {
        logger->error("连接失败: {}", e.what());
        isConnecting = false;
        handleStatusChange(ConnectionStatus::CHANNEL_ERROR, e.what());
    }
}

void WsConnectionManager::closeConnect() {
    handleStatusChange(ConnectionStatus::CLOSED, "主动关闭连接");

    if (wsSocket != INVALID_SOCKET) {
        closesocket(wsSocket);
        wsSocket = INVALID_SOCKET;
    }

    if (recvLoopThread.joinable()) {
        if (recvLoopThread.get_id() == std::this_thread::get_id()) {
            recvLoopThread.detach();
        } else {
            recvLoopThread.join();
        }
    }
}

void WsConnectionManager::autoReconnect() {
    if (reconnectCount >= maxReconnectTimes) {
        showErrorTip("无法恢复连接，请刷新页面");
        logger->error("重连尝试已达到最大次数 {}", maxReconnectTimes);
        return;
    }

    int delay = (reconnectCount < 3) ? RECONNECT_FAST_DELAY : RECONNECT_SLOW_DELAY;

    logger->info("将在 {} 毫秒后进行第 {}/{} 次重连",
                 delay, reconnectCount + 1, maxReconnectTimes);

    reconnectCount++;

    reconnectTask = scheduler->runDelayed([this]() {
        initConnect();
    }, std::chrono::milliseconds(delay));
}

void WsConnectionManager::OnStatusChange(std::function<void(ConnectionStatus, std::string)> callback) {
    statusChangeCallback = callback;
}

void WsConnectionManager::OnMessageReceived(std::function<void(std::string)> callback) {
    messageReceivedCallback = callback;
}

void WsConnectionManager::SendText(const std::string& text) {
    if (currentStatus != ConnectionStatus::SUBSCRIBED) {
        throw std::runtime_error("WebSocket is not subscribed");
    }

    WebSocketPacket packet;
    packet.set_fin(1);
    packet.set_opcode(WebSocketPacket::WSOpcode_Text);
    packet.set_payload(text.c_str(), text.size());

    ByteBuffer output;
    packet.pack_dataframe(output);
    networkSend(output.bytes(), output.length());
}

void WsConnectionManager::handleStatusChange(ConnectionStatus status, const std::string& errorMsg) {
    currentStatus = status;

    switch (status) {
        case ConnectionStatus::SUBSCRIBED:
            reconnectCount = 0;
            isConnecting = false;
            syncLoseMessage();
            startTimeoutCheck();
            if (statusChangeCallback) {
                statusChangeCallback(status, "");
            }
            break;

        case ConnectionStatus::CHANNEL_ERROR:
        case ConnectionStatus::TIMED_OUT:
            isConnecting = false;
            stopTimeoutCheck();
            autoReconnect();
            if (statusChangeCallback) {
                statusChangeCallback(status, errorMsg);
            }
            break;

        case ConnectionStatus::CLOSED:
            isConnecting = false;
            stopTimeoutCheck();
            if (reconnectTask) {
                reconnectTask->cancel();
                reconnectTask = nullptr;
            }
            if (statusChangeCallback) {
                statusChangeCallback(status, errorMsg);
            }
            break;
    }
}

void WsConnectionManager::syncLoseMessage() {
    std::string newMessageId = generateMessageId();

    if (!lastMessageId.empty()) {
        logger->info("同步断线期间丢失的消息，从 {} 到 {}",
                     lastMessageId, newMessageId);

        if (statusChangeCallback) {
            statusChangeCallback(ConnectionStatus::SUBSCRIBED,
                               "需要同步数据: " + lastMessageId);
        }
    }

    lastMessageId = newMessageId;
}

void WsConnectionManager::showConnectTip() {
    logger->info("连接已建立");
}

void WsConnectionManager::showErrorTip(const std::string& errorMsg) {
    logger->error("连接错误: {}", errorMsg);
}

int32_t WsConnectionManager::user_defined_process(WebSocketPacket& packet, ByteBuffer& frame_payload) {
    lastActivityTime = std::chrono::system_clock::now();

    switch (packet.get_opcode()) {
        case WebSocketPacket::WSOpcode_Text:
        case WebSocketPacket::WSOpcode_Binary: {
            std::string msg(frame_payload.bytes(), frame_payload.length());

            if (messageReceivedCallback) {
                messageReceivedCallback(msg);
            }

            if (currentStatus != ConnectionStatus::SUBSCRIBED) {
                handleStatusChange(ConnectionStatus::SUBSCRIBED);
            }
            break;
        }

        case WebSocketPacket::WSOpcode_Close:
            handleStatusChange(ConnectionStatus::CLOSED, "服务端主动关闭连接");
            break;

        case WebSocketPacket::WSOpcode_Ping: {
            WebSocketPacket pongPacket;
            pongPacket.set_fin(1);
            pongPacket.set_opcode(WebSocketPacket::WSOpcode_Pong);
            pongPacket.set_payload(frame_payload.bytes(), frame_payload.length());

            ByteBuffer output;
            pongPacket.pack_dataframe(output);
            networkSend(output.bytes(), output.length());
            break;
        }

        default:
            break;
    }

    return 0;
}

void WsConnectionManager::startTimeoutCheck() {
    stopTimeoutCheck();

    timeoutCheckTask = scheduler->runTask([this]() {
        if (currentStatus == ConnectionStatus::SUBSCRIBED) {
            auto now = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastActivityTime
            ).count();

            if (elapsed >= TIMEOUT_SECONDS) {
                logger->warning("连接超时，最后活动时间: {} 秒前", elapsed);
                handleStatusChange(ConnectionStatus::TIMED_OUT, "连接超时");
            }
        }
    }, std::chrono::seconds(5));
}

void WsConnectionManager::stopTimeoutCheck() {
    if (timeoutCheckTask) {
        timeoutCheckTask->cancel();
        timeoutCheckTask = nullptr;
    }
}

std::string WsConnectionManager::generateMessageId() {
    return tools::generate_pack_id();
}

std::string WsConnectionManager::generateMd5(const std::string& input) {
    return tools::generate_pack_id();
}

void WsConnectionManager::networkRecvLoop() {
    std::vector<char> buffer;
    buffer.reserve(8192);

    while (currentStatus == ConnectionStatus::SUBSCRIBED) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200 * 1000;

        fd_set fds_read;
        FD_ZERO(&fds_read);
        FD_SET(wsSocket, &fds_read);
        int ret = select((int)(wsSocket + 1), &fds_read, NULL, NULL, &tv);

        if (ret < 0) {
            handleStatusChange(ConnectionStatus::CHANNEL_ERROR, "socket select error");
            break;
        }

        if (ret == 0) {
            continue;
        }

        if (FD_ISSET(wsSocket, &fds_read)) {
            std::array<char, 4096> buf = {};
            int bytesReceived = recv(wsSocket, buf.data(), (int)buf.size(), 0);
            if (bytesReceived > 0) {
                from_wire(buf.data(), bytesReceived);
            } else {
                handleStatusChange(ConnectionStatus::CHANNEL_ERROR, "connection lost");
                break;
            }
        }
    }
}

bool WsConnectionManager::performHandshake(const std::string& hostname, int port, const std::string& path) {
    std::string handshakingWords;
    handshakingWords.append("GET ").append(path).append(" HTTP/1.1").append("\r\n");
    handshakingWords.append("Host: ").append(hostname).append(":").append(std::to_string(port)).append("\r\n");
    handshakingWords.append("Connection: Upgrade").append("\r\n");
    handshakingWords.append("Upgrade: websocket").append("\r\n");
    handshakingWords.append("Sec-WebSocket-Version: 13").append("\r\n");
    handshakingWords.append("Sec-WebSocket-Key: O7Tk4xI04v+X91cuvefLSQ==").append("\r\n");
    handshakingWords.append("\r\n");

    int sendResult = send(wsSocket, handshakingWords.c_str(), (int)handshakingWords.size(), 0);
    if (sendResult == SOCKET_ERROR) {
        logger->error("握手发送失败");
        return false;
    }

    char response_buffer[4096] = { 0 };
    int bytesReceived = recv(wsSocket, response_buffer, 4096, 0);
    if (bytesReceived <= 0) {
        logger->error("握手响应失败");
        return false;
    }

    std::string response(response_buffer, bytesReceived);
    if (response.find("101 Switching Protocols") == std::string::npos) {
        logger->error("握手失败: {}", response);
        return false;
    }

    return true;
}

void WsConnectionManager::networkSend(const char* data, int64_t size) {
    std::lock_guard<std::mutex> lock(sendMutex);
    int sendResult = send(wsSocket, data, (int)size, 0);
    if (sendResult == SOCKET_ERROR) {
        throw std::runtime_error("socket send error");
    }
}

socket_t WsConnectionManager::hostnameConnect(const std::string& hostname, int port) {
    struct addrinfo* result;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (ret != 0) {
        throw std::runtime_error("getaddrinfo failed.");
    }

    socket_t sockfd = INVALID_SOCKET;
    for (struct addrinfo* p = result; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET) { continue; }
        if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) != SOCKET_ERROR) {
            break;
        }
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }
    freeaddrinfo(result);
    return sockfd;
}