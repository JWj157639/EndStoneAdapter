#pragma once
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <cstring>

/**
 * 简单测试框架
 * 用于验收 EndStoneAdapter 新功能
 */

struct TestResult {
    std::string test_name;
    bool passed;
    std::string message;
};

class TestRunner {
private:
    std::vector<TestResult> results_;
    
public:
    void RunTest(const std::string& name, std::function<bool(std::string&)> test_fn);
    void PrintSummary() const;
    bool AllPassed() const;
};

// 测试断言宏
#define ASSERT_TRUE(expr) if (!(expr)) { msg = #expr " 为 false"; return false; }
#define ASSERT_FALSE(expr) if (expr) { msg = #expr " 为 true"; return false; }
#define ASSERT_EQ(a, b) if ((a) != (b)) { msg = #a " != " #b; return false; }
#define ASSERT_NE(a, b) if ((a) == (b)) { msg = #a " == " #b; return false; }
#define ASSERT_STR_EQ(a, b) if (std::strcmp((a), (b)) != 0) { msg = std::string(a) + " != " + std::string(b); return false; }

// 测试函数声明
bool TestAnsiStrip();
bool TestFilterTextByRegex();
bool TestContainsRegexMatch();
bool TestReplacePlaceholder();
bool TestPostEventConfig();
bool TestMotdMarkdownConfig();
bool TestFilterRegexListConfig();