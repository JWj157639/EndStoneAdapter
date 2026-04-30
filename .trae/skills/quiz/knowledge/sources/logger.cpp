// 防重复包含保护
#ifndef LOGGER_CPP
#define LOGGER_CPP

#include <windows.h>
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include "logger.h"

// 静态成员初始化
LogLevel Logger::s_currentLevel = LOG_LEVEL_INFO;
FILE* Logger::s_outputFile = stdout;
bool Logger::s_enableTimestamp = true;
CRITICAL_SECTION Logger::s_logMutex;
bool Logger::s_initialized = false;

#endif // LOGGER_CPP

void Logger::InitLogger(LogLevel level) {
    s_currentLevel = level;
    s_outputFile = stdout;
    s_enableTimestamp = true;
    InitializeCriticalSection(&s_logMutex);
    s_initialized = true;
}

void Logger::SetLogLevel(LogLevel level) {
    EnterCriticalSection(&s_logMutex);
    s_currentLevel = level;
    LeaveCriticalSection(&s_logMutex);
}

LogLevel Logger::GetLogLevel() {
    EnterCriticalSection(&s_logMutex);
    LogLevel level = s_currentLevel;
    LeaveCriticalSection(&s_logMutex);
    return level;
}

void Logger::LogMessage(LogLevel level, const char* format, ...) {
    if (!s_initialized) {
        return;
    }

    // 检查日志等级
    if (level < s_currentLevel) {
        return;
    }

    if (!format) {
        return;
    }

    va_list args;
    char buffer[1024];
    char timeStr[64];
    time_t rawtime;
    struct tm timeinfo;

    // 获取当前时间
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // 格式化日志消息
    va_start(args, format);
    int result = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
    va_end(args);
    
    if (result >= 0) {
        // 获取日志等级字符串
        const char* levelStr;
        switch (level) {
            case LOG_LEVEL_DEBUG: levelStr = "DEBUG"; break;
            case LOG_LEVEL_INFO:  levelStr = "INFO "; break;
            case LOG_LEVEL_WARN:  levelStr = "WARN "; break;
            case LOG_LEVEL_ERROR: levelStr = "ERROR"; break;
            case LOG_LEVEL_FATAL: levelStr = "FATAL"; break;
            default:              levelStr = "INFO "; break;
        }
        
        // 使用临界区保护输出
        EnterCriticalSection(&s_logMutex);
        
        // 根据日志等级使用不同颜色输出
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (level) {
            case LOG_LEVEL_DEBUG:
                SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
                break;
            case LOG_LEVEL_INFO:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            case LOG_LEVEL_WARN:
                SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
                break;
            case LOG_LEVEL_ERROR:
                SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
                break;
            case LOG_LEVEL_FATAL:
                SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
                break;
        }
        
        if (s_enableTimestamp) {
            fprintf(s_outputFile, "[%s] [%s] %s\n", timeStr, levelStr, buffer);
        } else {
            fprintf(s_outputFile, "[%s] %s\n", levelStr, buffer);
        }
        fflush(s_outputFile);
        
        // 恢复控制台默认颜色
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        
        LeaveCriticalSection(&s_logMutex);
    }
}

void Logger::Cleanup() {
    if (s_initialized) {
        DeleteCriticalSection(&s_logMutex);
        s_initialized = false;
    }
}

// 简化的初始化函数实现
void logger_init(LogLevel level) {
    Logger::InitLogger(level);
}