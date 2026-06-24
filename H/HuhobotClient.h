#pragma once
#include "quiz/WsConnectionManager.h"
#include "endstone/logger.h"
#include "nlohmann/json.hpp"
#include "tools.h"
#include <unordered_map>
#include "endstone/scheduler/task.h"
#include "config.h"

using endstone::Logger;
using json = nlohmann::json;

enum class ServerRecvEvent;
enum class ServerSendEvent;

struct EnumConverter {
    static std::string ToString(ServerSendEvent e);
    static std::string ToString(ServerRecvEvent e);
    static ServerRecvEvent FromString(const std::string& str);
};

class BotClient{
private:
    std::unique_ptr<WsConnectionManager> wsManager;
    std::string serverUrl = HUHOBOT_SERVER_URL;
    Logger* logger;
    std::unordered_map<std::string, std::string> bindMap;
    bool shouldReconnect;
    bool isShaked = false;
    std::shared_ptr<endstone::Task> heartTask = nullptr;
    std::shared_ptr<endstone::Task> autoDisConnectTask = nullptr;

    json buildMsg(ServerSendEvent event_type,json body,std::string packId);
    void shakeHand();
    void shakedProcess();
    void processMessage(const std::string& msg);

    //Event Handler
    void handler_sendConfig(std::string packId,json &body);
    void handler_chat(std::string packId,json &body);
    void handler_add(std::string packId,json &body);
    void handler_delete_(std::string packId,json &body);
    void handler_cmd(std::string packId,json &body);
    void handler_queryList(std::string packId,json &body);
    void handler_queryOnline(std::string packId,json &body);
    void handler_shutdown(std::string packId,json &body);
    void handler_run(std::string packId,json &body,bool isAdmin);
    void handler_heart(std::string packId,json &body);
    void handler_bindRequest(std::string packId,json &body);
    void handler_shaked(std::string packId,json &body);

public:
    BotClient(Logger* logger, endstone::Plugin* plugin);
    void connect();
    void sendMessage(
            ServerSendEvent event_type,
            json& body,
            string packId=tools::generate_pack_id()
    );
    void bindConfirm(string code);
    void sendHeart();
    void sendChat(string msg);
    void shutdown(bool _shouldReconnect=true);
    void reconnect();

    /**
     * 推送玩家进出事件消息
     */
    void postPlayerEvent(const std::string& playerName, bool isJoin);
};