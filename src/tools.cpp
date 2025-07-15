#include "tools.h"
#include <random>
#include <sstream>
#include <algorithm>

// UUID版本4生成器（符合RFC 4122）
std::string tools::generate_pack_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;

    // 生成16字节（32个十六进制字符）
    for (int i=0; i<32; ++i) {
        if (i == 12) { // 设置版本号4
            ss << 4;
        }
        else if (i == 16) { // 设置变体位
            int r = dis2(gen);
            ss << std::hex << r;
        }
        else {
            int r = dis(gen);
            ss << std::hex << r;
        }

        // 按UUID格式插入分隔符（后续会移除）
        if (i == 7 || i == 11 || i == 15 || i == 19) {
            ss << "-";
        }
    }

    std::string uuid = ss.str();
    uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'), uuid.end());
    return uuid;
}

// 字符串分割工具
std::vector<std::string> tools::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// 集合过滤工具
template<typename T>
std::vector<T> tools::filterSet(const std::set<T>& original,
                         std::function<bool(const T&)> predicate) {
    std::vector<T> result;
    std::copy_if(original.begin(), original.end(),
                 std::back_inserter(result), predicate);
    return result;
}

std::string& replaceAll(std::string& str, std::string_view oldValue, std::string_view newValue) {
    for (std::string::size_type pos(0); pos != std::string::npos; pos += newValue.length()) {
        if ((pos = str.find(oldValue, pos)) != std::string::npos) str.replace(pos, oldValue.length(), newValue);
        else break;
    }
    return str;
}
std::string replaceAll(std::string const& str, std::string_view oldValue, std::string_view newValue) {
    std::string ret = str;
    replaceAll(ret, oldValue, newValue);
    return ret;
}

std::wstring str2wstr(std::string_view str, unsigned int codePage) {
    int          len = MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), nullptr, 0);
    std::wstring wstr;
    if (len == 0) return wstr;
    wstr.resize(len);
    MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), wstr.data(), len);
    return wstr;
}
std::string wstr2str(std::wstring_view wstr, unsigned int codePage) {
    int         len = WideCharToMultiByte(codePage, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str;
    if (len == 0) return str;
    str.resize(len);
    WideCharToMultiByte(codePage, 0, wstr.data(), (int)wstr.size(), str.data(), (int)str.size(), nullptr, nullptr);
    return str;
}