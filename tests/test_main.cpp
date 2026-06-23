#include "TestRunner.h"
#include "TextUtils.h"
#include "ConfigManager.h"
#include <iostream>
#include <sstream>

void TestRunner::RunTest(const std::string& name, std::function<bool(std::string&)> test_fn) {
    std::string msg;
    bool passed = test_fn(msg);
    results_.push_back({name, passed, msg});
    
    if (passed) {
        std::cout << "[PASS] " << name << std::endl;
    } else {
        std::cout << "[FAIL] " << name << " - " << msg << std::endl;
    }
}

void TestRunner::PrintSummary() const {
    std::cout << "\n========== 测试总结 ==========" << std::endl;
    int passed = 0, failed = 0;
    for (const auto& r : results_) {
        if (r.passed) passed++;
        else failed++;
    }
    std::cout << "总计: " << results_.size() << " 个测试" << std::endl;
    std::cout << "通过: " << passed << std::endl;
    std::cout << "失败: " << failed << std::endl;
    std::cout << "==============================" << std::endl;
}

bool TestRunner::AllPassed() const {
    for (const auto& r : results_) {
        if (!r.passed) return false;
    }
    return true;
}

// ========== 文本过滤功能测试 ==========

bool TestAnsiStrip() {
    std::string msg;
    
    // 测试1: 基本 ANSI 颜色代码
    std::string with_ansi = "\x1B[31m红色文本\x1B[0m";
    std::string stripped = TextUtils::StripAnsiEscape(with_ansi);
    ASSERT_STR_EQ(stripped.c_str(), "红色文本");
    
    // 测试2: 多个 ANSI 序列
    std::string multi_ansi = "\x1B[1;32m绿色粗体\x1B[0m 普通 \x1B[4m下划线\x1B[0m";
    std::string multi_stripped = TextUtils::StripAnsiEscape(multi_ansi);
    ASSERT_STR_EQ(multi_stripped.c_str(), "绿色粗体 普通 下划线");
    
    // 测试3: 无 ANSI 序列的文本
    std::string no_ansi = "普通文本";
    std::string no_ansi_stripped = TextUtils::StripAnsiEscape(no_ansi);
    ASSERT_STR_EQ(no_ansi_stripped.c_str(), "普通文本");
    
    // 测试4: 空字符串
    std::string empty = "";
    std::string empty_stripped = TextUtils::StripAnsiEscape(empty);
    ASSERT_STR_EQ(empty_stripped.c_str(), "");
    
    return true;
}

bool TestFilterTextByRegex() {
    std::string msg;
    
    // 测试1: 过滤 ANSI + 自定义正则
    std::vector<std::string> patterns = {"\\[.*?\\]"};  // 过滤方括号内容
    std::string text = "\x1B[31m[服务器] 消息\x1B[0m";
    std::string filtered = TextUtils::FilterTextByRegex(text, patterns, true);
    ASSERT_STR_EQ(filtered.c_str(), " 消息");
    
    // 测试2: 不过滤 ANSI
    std::string filtered_no_ansi = TextUtils::FilterTextByRegex(text, patterns, false);
    // ANSI 序列还在，但方括号内容被过滤
    ASSERT_TRUE(filtered_no_ansi.find("[服务器]") == std::string::npos);
    
    // 测试3: 空模式列表
    std::vector<std::string> empty_patterns;
    std::string text2 = "测试文本";
    std::string filtered_empty = TextUtils::FilterTextByRegex(text2, empty_patterns, false);
    ASSERT_STR_EQ(filtered_empty.c_str(), "测试文本");
    
    return true;
}

bool TestContainsRegexMatch() {
    std::string msg;
    
    // 测试1: 匹配成功
    std::vector<std::string> patterns = {"错误", "异常", "失败"};
    std::string text = "服务器发生错误";
    ASSERT_TRUE(TextUtils::ContainsRegexMatch(text, patterns, false));
    
    // 测试2: 匹配失败
    std::string text2 = "服务器正常运行";
    ASSERT_FALSE(TextUtils::ContainsRegexMatch(text2, patterns, false));
    
    // 测试3: ANSI 序列干扰
    std::string text3 = "\x1B[31m错误\x1B[0m";
    ASSERT_TRUE(TextUtils::ContainsRegexMatch(text3, patterns, true));
    
    return true;
}

bool TestReplacePlaceholder() {
    std::string msg;
    
    // 测试1: 基本占位符替换
    std::string template1 = "{playerName} 加入了服务器";
    std::string result1 = TextUtils::ReplacePlaceholder(template1, "playerName", "Steve");
    ASSERT_STR_EQ(result1.c_str(), "Steve 加入了服务器");
    
    // 测试2: 多个相同占位符
    std::string template2 = "{playerName} 欢迎 {playerName}";
    std::string result2 = TextUtils::ReplacePlaceholder(template2, "playerName", "Alex");
    ASSERT_STR_EQ(result2.c_str(), "Alex 欢迎 Alex");
    
    // 测试3: 无占位符
    std::string template3 = "固定消息";
    std::string result3 = TextUtils::ReplacePlaceholder(template3, "playerName", "Steve");
    ASSERT_STR_EQ(result3.c_str(), "固定消息");
    
    // 测试4: 空值替换
    std::string template4 = "{playerName} 离开了";
    std::string result4 = TextUtils::ReplacePlaceholder(template4, "playerName", "");
    ASSERT_STR_EQ(result4.c_str(), " 离开了");
    
    return true;
}

// ========== 配置管理测试 ==========

bool TestPostEventConfig() {
    std::string msg;
    
    auto& config = ConfigManager::Get();
    
    auto joinConfig = config.GetPostEventOnJoin();
    ASSERT_FALSE(joinConfig.enable);
    ASSERT_STR_EQ(joinConfig.formatString.c_str(), "玩家 {playerName} 加入了服务器");
    
    auto leftConfig = config.GetPostEventOnLeft();
    ASSERT_FALSE(leftConfig.enable);
    ASSERT_STR_EQ(leftConfig.formatString.c_str(), "玩家 {playerName} 离开了服务器");
    
    return true;
}

bool TestMotdMarkdownConfig() {
    std::string msg;
    
    auto& config = ConfigManager::Get();
    
    auto motdConfig = config.GetMotdConfig();
    ASSERT_FALSE(motdConfig.useMarkdown);
    ASSERT_FALSE(motdConfig.customMarkdown);
    ASSERT_STR_EQ(motdConfig.text.c_str(), "共{online}人在线");
    
    return true;
}

bool TestFilterRegexListConfig() {
    std::string msg;
    
    auto& config = ConfigManager::Get();
    
    auto patterns = config.GetFilterRegexList();
    ASSERT_TRUE(!patterns.empty());
    ASSERT_TRUE(patterns.size() >= 1);
    
    return true;
}

// ========== 主函数 ==========

int main() {
    std::cout << "========== EndStoneAdapter 功能测试 ==========" << std::endl;
    std::cout << "测试日期: 2026-06-23" << std::endl;
    std::cout << "=============================================\n" << std::endl;
    
    TestRunner runner;
    
    // 文本过滤功能测试
    std::cout << "--- 文本过滤功能测试 ---" << std::endl;
    runner.RunTest("ANSI转义序列清理", [](std::string& msg) { return TestAnsiStrip(); });
    runner.RunTest("正则表达式过滤", [](std::string& msg) { return TestFilterTextByRegex(); });
    runner.RunTest("正则表达式匹配检测", [](std::string& msg) { return TestContainsRegexMatch(); });
    runner.RunTest("占位符替换", [](std::string& msg) { return TestReplacePlaceholder(); });
    
    // 配置管理测试
    std::cout << "\n--- 配置管理测试 ---" << std::endl;
    runner.RunTest("玩家进出事件配置", [](std::string& msg) { return TestPostEventConfig(); });
    runner.RunTest("MOTD Markdown配置", [](std::string& msg) { return TestMotdMarkdownConfig(); });
    runner.RunTest("过滤正则列表配置", [](std::string& msg) { return TestFilterRegexListConfig(); });
    
    runner.PrintSummary();
    
    return runner.AllPassed() ? 0 : 1;
}