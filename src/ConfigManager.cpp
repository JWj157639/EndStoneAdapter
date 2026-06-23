// ConfigManager.cpp
#include "ConfigManager.h"
#include <fstream>
#include <regex>
#include "tools.h" // 包含generate_pack_id()


ConfigManager& ConfigManager::Get() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    Load("plugins/HuHoBot/config.json");
}

void ConfigManager::Reload() {
    Load(path_); // 使用之前保存的路径重新加载
}

void ConfigManager::Load(const std::string& path) {
    path_ = path;

    try {
        std::ifstream fin(path);
        if (fin.good()) {
            data_ = json::parse(fin);
        }
    } catch (...) {
        // 文件不存在或解析失败
    }

    InitDefaults();
    Save();
}

void ConfigManager::Save() {
    // 创建父目录
    auto config_path = std::filesystem::path(path_);
    if (!std::filesystem::exists(config_path.parent_path())) {
        std::filesystem::create_directories(config_path.parent_path());
    }

    std::ofstream fout(path_);
    if (fout.is_open()) {
        fout << data_.dump(4);
        fout.close(); // 确保数据刷新到磁盘
    }
}

void ConfigManager::InitDefaults() {
    // 版本号自动升级
    if (!data_.contains("version") || data_["version"] < 3) {

        data_["version"] = version_;
    }

    // 旧版本配置迁移
    if (data_["version"] == 1) {

        // 迁移 chatFormatGroup 到新结构
        if (data_.contains("chatFormatGroup")) {
            data_["chatFormat"]["from_group"] = data_["chatFormatGroup"];
            data_.erase("chatFormatGroup");
        }
        data_["version"] = version_; // 升级版本号
    }

    if (!data_.contains("serverId") || data_["serverId"].empty()) {
        data_["serverId"] = tools::generate_pack_id();
    }

    if (!data_.contains("hashKey") || data_["hashKey"].empty()) {
        data_["hashKey"] = "";
    }

    // 初始化聊天格式配置
    if (!data_.contains("chatFormat")) {
        data_["chatFormat"] = {
                {"from_game", "<{name}> {msg}"},
                {"from_group", "群:<{nick}> {msg}"},
                {"post_chat", true},
                {"post_prefix", ""}
        };
    } else {
        // 确保所有子字段存在
        auto& chatFormat = data_["chatFormat"];
        if (!chatFormat.contains("from_game")) chatFormat["from_game"] = "<{name}> {msg}";
        if (!chatFormat.contains("from_group")) chatFormat["from_group"] = "群:<{nick}> {msg}";
        if (!chatFormat.contains("post_chat")) chatFormat["post_chat"] = true;
        if (!chatFormat.contains("post_prefix")) chatFormat["post_prefix"] = "";
    }

    if (!data_.contains("motdUrl")) {
        data_["motdUrl"] = "play.easecation.net:19132";
    }

    if (!data_.contains("serverName")) {
        data_["serverName"] = "EndStone";
    }

    if (!data_.contains("customCommand")) {
        data_["customCommand"] = std::vector<CustomCommand>{
                {"加白名", "whitelist add &1", 0},
                {"管理加白名", "whitelist add &1", 1}
        };
    }

    if(!data_.contains("callbackConvertImg")){
        data_["callbackConvertImg"] = 0;
    }

    if (!data_.contains("maxReconnectTimes")) {
        data_["maxReconnectTimes"] = 10;
    }

    // 初始化 postEvent 配置
    if (!data_.contains("postEvent")) {
        data_["postEvent"] = {
            {"onJoin", {{"enable", false}, {"formatString", "玩家 {playerName} 加入了服务器"}}},
            {"onLeft", {{"enable", false}, {"formatString", "玩家 {playerName} 离开了服务器"}}}
        };
    } else {
        if (!data_["postEvent"].contains("onJoin")) {
            data_["postEvent"]["onJoin"] = {{"enable", false}, {"formatString", "玩家 {playerName} 加入了服务器"}};
        }
        if (!data_["postEvent"].contains("onLeft")) {
            data_["postEvent"]["onLeft"] = {{"enable", false}, {"formatString", "玩家 {playerName} 离开了服务器"}};
        }
    }

    // 初始化 motd 配置
    if (!data_.contains("motd")) {
        data_["motd"] = {
            {"server_ip", "play.easecation.net"},
            {"server_port", 19132},
            {"api", "https://motdbe.blackbe.work/status_img?host={server_ip}:{server_port}"},
            {"text", "共{online}人在线"},
            {"output_online_list", true},
            {"post_img", true},
            {"markdown", true},
            {"customMarkdown", false}
        };
    } else {
        auto& motd = data_["motd"];
        if (!motd.contains("server_ip")) motd["server_ip"] = "play.easecation.net";
        if (!motd.contains("server_port")) motd["server_port"] = 19132;
        if (!motd.contains("api")) motd["api"] = "https://motdbe.blackbe.work/status_img?host={server_ip}:{server_port}";
        if (!motd.contains("text")) motd["text"] = "共{online}人在线";
        if (!motd.contains("output_online_list")) motd["output_online_list"] = true;
        if (!motd.contains("post_img")) motd["post_img"] = true;
        if (!motd.contains("markdown")) motd["markdown"] = true;
        if (!motd.contains("customMarkdown")) motd["customMarkdown"] = false;
    }

    // 初始化 whiteList 配置
    if (!data_.contains("whiteList")) {
        data_["whiteList"] = {
            {"add", "whitelist add {name}"},
            {"del", "whitelist remove {name}"}
        };
    } else {
        auto& whiteList = data_["whiteList"];
        if (!whiteList.contains("add")) whiteList["add"] = "whitelist add {name}";
        if (!whiteList.contains("del")) whiteList["del"] = "whitelist remove {name}";
    }

    // 初始化 redis 配置
    if (!data_.contains("redis")) {
        data_["redis"] = {
            {"enabled", false},
            {"host", "localhost"},
            {"port", 6379},
            {"password", ""},
            {"channel", "HuHoBotChannel"}
        };
    } else {
        auto& redis = data_["redis"];
        if (!redis.contains("enabled")) redis["enabled"] = false;
        if (!redis.contains("host")) redis["host"] = "localhost";
        if (!redis.contains("port")) redis["port"] = 6379;
        if (!redis.contains("password")) redis["password"] = "";
        if (!redis.contains("channel")) redis["channel"] = "HuHoBotChannel";
    }

    // 初始化 name 配置
    if (!data_.contains("name") || data_["name"].empty()) {
        data_["name"] = "HuHoBot";
    }

    // 初始化 filterRegexList
    if (!data_.contains("filterRegexList")) {
        data_["filterRegexList"] = std::vector<std::string>{"\\u001B\\[[;\\d]*[ -/]*[@-~]"};
    }
}

// Getter实现
int ConfigManager::GetVersion() const {
    return data_["version"];
}

std::string ConfigManager::GetServerId() const {
    return data_["serverId"];
}

std::string ConfigManager::GetHashKey() const {
    return data_["hashKey"];
}

std::string ConfigManager::GetChatFormatFromGame() const {
    return data_["chatFormat"]["from_game"];
}

std::string ConfigManager::GetChatFormatFromGroup() const {
    return data_["chatFormat"]["from_group"];
}

bool ConfigManager::GetPostChat() const {
    return data_["chatFormat"]["post_chat"];
}

std::string ConfigManager::GetPostPrefix() const {
    return data_["chatFormat"]["post_prefix"];
}

std::string ConfigManager::GetMotdUrl() const {
    return data_["motdUrl"];
}

std::string ConfigManager::GetServerName() const {
    return data_["serverName"];
}

std::vector<CustomCommand> ConfigManager::GetCustomCommands() const {
    return data_["customCommand"];
}

int ConfigManager::GetCallbackConvertImg() const {
    return data_["callbackConvertImg"];
}

int ConfigManager::GetMaxReconnectTimes() const {
    return data_.value("maxReconnectTimes", 10);
}

// Setter实现
void ConfigManager::SetServerId(const std::string& id) {
    data_["serverId"] = id;
}

void ConfigManager::SetHashKey(const std::string& key) {
    data_["hashKey"] = key;
}

PostEventEntry ConfigManager::GetPostEventOnJoin() const {
    return data_["postEvent"]["onJoin"].get<PostEventEntry>();
}

PostEventEntry ConfigManager::GetPostEventOnLeft() const {
    return data_["postEvent"]["onLeft"].get<PostEventEntry>();
}

MotdConfig ConfigManager::GetMotdConfig() const {
    return data_["motd"].get<MotdConfig>();
}

WhiteListConfig ConfigManager::GetWhiteListConfig() const {
    return data_["whiteList"].get<WhiteListConfig>();
}

RedisConfig ConfigManager::GetRedisConfig() const {
    return data_["redis"].get<RedisConfig>();
}

std::vector<std::string> ConfigManager::GetFilterRegexList() const {
    return data_["filterRegexList"].get<std::vector<std::string>>();
}

std::string ConfigManager::GetName() const {
    return data_.value("name", "HuHoBot");
}