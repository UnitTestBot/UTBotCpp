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
    bool isNumber(std::string_view s);

    bool startsWith(std::string_view str, std::string_view pattern);

    bool endsWith(std::string_view str, std::string_view pattern);

    std::string tolower(std::string_view data);

    std::string toupper(std::string_view data);

    std::vector<std::string> split(std::string_view str, char delim = ' ');

    template <typename Container = std::initializer_list<std::string>>
    std::string joinWith(Container &&items, std::string_view del) {
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

    bool replaceLast(std::string &str, const std::string &from, const std::string &to);

    bool replaceFirst(std::string &str, const std::string &from, const std::string &to);

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

    std::vector<std::string> splitByWhitespaces(std::string_view input);

    void removeLineEndings(std::string &s);

    void ltrim(std::string &s);

    void rtrim(std::string &s);

    void trim(std::string &s);

    std::string repeat(const std::string &s, int n);

    bool contains(std::string_view s, std::string_view t);

    template<typename T>
    T stot(const std::string&) {
        return T();
    }
}

#endif //UNITTESTBOT_STRINGUTIL_H
