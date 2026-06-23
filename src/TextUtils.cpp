#include "TextUtils.h"
#include <sstream>

namespace TextUtils {

std::string StripAnsiEscape(const std::string& text) {
    static const std::regex ansi_regex("\x1B\\[[;\\d]*[ -/]*[@-~]");
    return std::regex_replace(text, ansi_regex, "");
}

std::string FilterTextByRegex(const std::string& text, const std::vector<std::string>& patterns, bool removeAnsiEscape) {
    std::string result = removeAnsiEscape ? StripAnsiEscape(text) : text;

    for (const auto& pattern : patterns) {
        try {
            std::regex re(pattern);
            result = std::regex_replace(result, re, "");
        } catch (const std::regex_error&) {
            // 跳过无效的正则表达式
        }
    }

    return result;
}

bool ContainsRegexMatch(const std::string& text, const std::vector<std::string>& patterns, bool removeAnsiEscape) {
    std::string processed = removeAnsiEscape ? StripAnsiEscape(text) : text;

    for (const auto& pattern : patterns) {
        try {
            std::regex re(pattern);
            if (std::regex_search(processed, re)) {
                return true;
            }
        } catch (const std::regex_error&) {
            // 跳过无效的正则表达式
        }
    }

    return false;
}

std::string ReplacePlaceholder(const std::string& text, const std::string& placeholder, const std::string& replacement) {
    std::string result = text;
    size_t pos = 0;
    while ((pos = result.find(placeholder, pos)) != std::string::npos) {
        result.replace(pos, placeholder.length(), replacement);
        pos += replacement.length();
    }
    return result;
}

} // namespace TextUtils