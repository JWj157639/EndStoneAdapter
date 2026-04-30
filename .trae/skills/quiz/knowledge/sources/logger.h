#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <stdio.h>

/**
 * @brief 日志等级枚举
 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_FATAL = 4
} LogLevel;

/**
 * @brief 日志类
 * 
 * 提供多等级日志记录功能，支持颜色输出和时间戳
 */
class Logger {
public:
    /**
     * @brief 初始化日志系统
     * @param level 初始日志等级
     */
    static void InitLogger(LogLevel level);

    /**
     * @brief 设置日志等级
     * @param level 新的日志等级
     */
    static void SetLogLevel(LogLevel level);

    /**
     * @brief 获取当前日志等级
     * @return 当前日志等级
     */
    static LogLevel GetLogLevel();

    /**
     * @brief 记录日志
     * @param level 日志等级
     * @param format 格式化字符串
     */
    static void LogMessage(LogLevel level, const char* format, ...);

    /**
     * @brief 清理日志系统
     */
    static void Cleanup();

private:
    static LogLevel s_currentLevel;
    static FILE* s_outputFile;
    static bool s_enableTimestamp;
    static CRITICAL_SECTION s_logMutex;
    static bool s_initialized;

    Logger() = delete;  // 禁止实例化
};

// 便捷的日志宏
#define LOG_DEBUG(fmt, ...) Logger::LogMessage(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger::LogMessage(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger::LogMessage(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::LogMessage(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger::LogMessage(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)

// 便捷的C风格接口
void logger_init(LogLevel level);

// 内联函数提供简化接口
inline void logger_set_level(LogLevel level) {
    Logger::SetLogLevel(level);
}

inline LogLevel logger_get_level() {
    return Logger::GetLogLevel();
}

inline void logger_cleanup() {
    Logger::Cleanup();
}

#endif // LOGGER_H