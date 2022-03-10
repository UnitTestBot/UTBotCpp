/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_STRINGUTIL_H
#define UNITTESTBOT_STRINGUTIL_H

#include "StringFormat.h"
#include "CollectionUtils.h"
#include "utils/path/FileSystemPath.h"

#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace StringUtils {
    using std::string;
    using std::string_view;
    using std::vector;

    bool isNumber(string_view s);

    bool startsWith(string_view str, string_view pattern);

    bool endsWith(string_view str, string_view pattern);

    string tolower(string_view data);

    string toupper(string_view data);

    vector<string> split(string_view str, char delim = ' ');

    template <typename Container = std::initializer_list<std::string>>
    string joinWith(Container &&items, string_view del) {
        std::stringstream ss;
        for (auto it = items.begin(); it != items.end(); it++) {
            using T = std::decay_t<decltype(*it)>;
            if constexpr (std::is_base_of_v<std::exception, T>) {
                ss << it->what();
            } else if constexpr (std::is_same_v<fs::path, T>){
                ss << it->string();
            } else {
                ss << *it;
            }
            if (std::next(it) != items.end()) {
                ss << del;
            }
        }
        return ss.str();
    }

    bool replaceLast(string &str, const string &from, const string &to);

    void replaceAll(std::string &str, char from, char to);

    void replaceAll(std::string &str, const std::string &from, const std::string &to);

    /**
     * Returns true if char literal can be printed to .cpp file as is, false otherwise.
     * @param value - given character code
     * @return whether printable or not
     */
    bool isPrintable(int code);

    /**
     * Returns the representation of character to C++ character literal
     * @param value - given character code
     * @return literal representation
     */
    std::string charCodeToLiteral(int code);

    std::vector<std::string> splitByWhitespaces(string_view input);

    void removeLineEndings(string& s);

    void ltrim(string &s);

    void rtrim(string &s);

    void trim(string &s);

    string repeat(const string &s, int n);

    bool contains(string_view s, string_view t);
}

#endif //UNITTESTBOT_STRINGUTIL_H
