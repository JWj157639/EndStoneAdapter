// ConfigManager.h
#pragma once
#include "nlohmann/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

/**
 * 自定义命令结构体
 * 用于存储自定义命令的配置信息
 */
struct CustomCommand {
    std::string key;        // 命令键
    std::string command;    // 命令内容
    int permission;         // 权限等级
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(CustomCommand, key, command, permission)
};


/**
 * 配置管理器
 * 
 * 负责加载、保存和管理应用程序配置
 * 使用单例模式确保全局只有一个配置管理器实例
 */
class ConfigManager {
public:
    /**
     * 获取配置管理器单例实例
     * @return 配置管理器实例的引用
     */
    static ConfigManager& Get();

    /**
     * 获取配置版本号
     * @return 配置版本号
     */
    int GetVersion() const;
    
    /**
     * 获取服务器 ID
     * @return 服务器 ID 字符串
     */
    std::string GetServerId() const;
    
    /**
     * 获取哈希密钥
     * @return 哈希密钥字符串
     */
    std::string GetHashKey() const;
    
    /**
     * 获取游戏内聊天格式
     * @return 聊天格式字符串
     */
    std::string GetChatFormatFromGame() const;
    
    /**
     * 获取群聊聊天格式
     * @return 聊天格式字符串
     */
    std::string GetChatFormatFromGroup() const;
    
    /**
     * 获取是否发布聊天消息
     * @return 是否发布聊天消息
     */
    bool GetPostChat() const;
    
    /**
     * 获取发布消息前缀
     * @return 消息前缀字符串
     */
    std::string GetPostPrefix() const;
    
    /**
     * 获取 MOTD（每日消息）URL
     * @return MOTD URL 字符串
     */
    std::string GetMotdUrl() const;
    
    /**
     * 获取服务器名称
     * @return 服务器名称字符串
     */
    std::string GetServerName() const;
    
    /**
     * 获取自定义命令列表
     * @return 自定义命令向量
     */
    std::vector<CustomCommand> GetCustomCommands() const;
    
    /**
     * 获取图片回调转换设置
     * @return 图片回调转换设置值
     */
    int GetCallbackConvertImg() const;
    
    /**
     * 获取最大重连次数
     * @return 最大重连次数
     */
    int GetMaxReconnectTimes() const;

    /**
     * 设置服务器 ID
     * @param id 服务器 ID
     */
    void SetServerId(const std::string& id);
    
    /**
     * 设置哈希密钥
     * @param key 哈希密钥
     */
    void SetHashKey(const std::string& key);

    /**
     * 从指定路径加载配置文件
     * @param path 配置文件路径
     */
    void Load(const std::string& path);
    
    /**
     * 重新加载配置文件
     */
    void Reload();
    
    /**
     * 保存配置到文件
     */
    void Save();

    /**
     * 构造函数
     */
    ConfigManager();

private:
    /**
     * 初始化默认配置值
     */
    void InitDefaults();

    json data_;            // 配置数据（JSON 格式）
    std::string path_;     // 配置文件路径
    int version_ = 3;      // 配置版本号
};