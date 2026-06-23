#pragma once
#include <string>
#include <vector>
#include <regex>

namespace TextUtils {

/**
 * 清理 ANSI 转义序列
 */
std::string StripAnsiEscape(const std::string& text);

/**
 * 根据正则表达式列表过滤文本
 * @param text 原始文本
 * @param patterns 正则表达式列表
 * @param removeAnsiEscape 是否先清理 ANSI 转义序列
 * @return 过滤后的文本
 */
std::string FilterTextByRegex(const std::string& text, const std::vector<std::string>& patterns, bool removeAnsiEscape = true);

/**
 * 检查文本是否匹配任何正则表达式
 * @param text 原始文本
 * @param patterns 正则表达式列表
 * @param removeAnsiEscape 是否先清理 ANSI 转义序列
 * @return 是否匹配
 */
bool ContainsRegexMatch(const std::string& text, const std::vector<std::string>& patterns, bool removeAnsiEscape = true);

/**
 * 替换占位符
 * @param text 原始文本
 * @param placeholder 占位符（如 "{playerName}"）
 * @param replacement 替换值
 * @return 替换后的文本
 */
std::string ReplacePlaceholder(const std::string& text, const std::string& placeholder, const std::string& replacement);

} // namespace TextUtils