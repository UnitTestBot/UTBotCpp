/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "StringUtils.h"
#include "PrinterUtils.h"

namespace StringUtils {
    bool isNumber(string_view s) {
        return !s.empty() && std::all_of(s.begin(),
                                         s.end(), [](unsigned char c) { return std::isdigit(c); });
    }

    bool startsWith(string_view str, string_view pattern) {
        if (str.length() >= pattern.length()) {
            return str.compare(0, pattern.length(), pattern) == 0;
        }
        return false;
    }

    bool endsWith(string_view str, string_view pattern) {
        if (str.length() >= pattern.length()) {
            return str.compare(str.length() - pattern.length(), pattern.length(), pattern) == 0;
        }
        return false;
    }

    string tolower(string_view data) {
        string res(data.length(), 0);
        std::transform(data.begin(), data.end(), std::begin(res),
                       [](unsigned char c) { return std::tolower(c); });
        return res;
    }

    string toupper(string_view data) {
        string res(data.length(), 0);
        std::transform(data.begin(), data.end(), std::begin(res),
                       [](unsigned char c) { return std::toupper(c); });
        return res;
    }


    vector<string> split(string_view str, char delim) {
        vector<string> result;
        std::stringstream ss(str.data());
        string token;
        while (std::getline(ss, token, delim)) {
            result.push_back(token);
        }
        return result;
    }

    bool replaceLast(string &str, const string &from, const string &to) {
        size_t start_pos = str.rfind(from);
        if (start_pos != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            return true;
        } else {
            return false;
        }
    }

    void replaceAll(string &str, char from, char to) {
        for (char &c : str) {
            if (c == from) {
                c = to;
            }
        }
    }

    void replaceAll(string &str, const string &from, const string &to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    bool isPrintable(int code) {
        if (std::numeric_limits<char>::is_signed && code < 0) {
            code += 256;
        }
        return !CollectionUtils::contains(PrinterUtils::escapeSequences, code) && std::isprint(code);
    }

    std::string charCodeToLiteral(int code) {
        if (std::numeric_limits<char>::is_signed && code < 0) {
            code += 256;
        }
        std::string res;
        if (CollectionUtils::contains(PrinterUtils::escapeSequences, code)) {
            res += PrinterUtils::escapeSequences.at(code);
        } else if (std::isprint(code)) {
            res += static_cast<char>(code);
        } else {
            std::stringstream ss;
            ss << std::hex << code;
            std::string hexValue = ss.str();
            std::string pad(std::max(0, 2 - (int) hexValue.size()), '0');
            res += "\\x" + pad + hexValue;
        }
        return res;
    }

    std::vector<std::string> splitByWhitespaces(string_view input) {
        std::istringstream buffer(input.data());
        std::vector<std::string> ret((std::istream_iterator<std::string>(buffer)),
                                     std::istream_iterator<std::string>());
        return ret;
    }

    void ltrim(string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                    return std::isspace(ch) == 0;
                }));
    }

    void rtrim(string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                    return std::isspace(ch) == 0;
                }).base(),
                s.end());
    }

    void trim(string &s) {
        ltrim(s);
        rtrim(s);
    }

    string repeat(const string &s, int n) {
        string s1;
        s1.reserve(s.size() * std::max(0, n));
        for (int i = 0; i < n; i++) {
            s1 += s;
        }
        return s1;
    }

    bool contains(string_view s, string_view t) {
        return s.find(t) != string_view::npos;
    }

    void removeLineEndings(string &s) {
        s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    }

}