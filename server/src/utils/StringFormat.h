#ifndef UNITTESTBOT_STRINGFORMAT_H
#define UNITTESTBOT_STRINGFORMAT_H

#include <exception>
#include <experimental/type_traits>
#include <stdexcept>
#include <string>
#include <vector>

namespace StringUtils {
    extern thread_local std::vector<char> formatBuffer;

    namespace internal {
        template <typename T>
        using has_c_str = decltype(std::declval<T>().c_str());

        template <typename Arg>
        decltype(auto) extractCString(Arg &&arg) {
            if constexpr (std::experimental::is_detected_v<has_c_str, Arg>) {
                return std::forward<Arg>(arg).c_str();
            } else {
                return std::forward<Arg>(arg);
            }
        }
    }

    /**
     * @brief concatenate arguments according to the format
     * @param format format
     * @param args each of elements must either be primitive or have c_str method
     * (e.g. std::string, fs::path and others)
     */
    template <typename... Args>
    std::string stringFormat(const std::string &format, Args&&... args) {
        size_t size = snprintf(nullptr, 0, format.c_str(), internal::extractCString(std::forward<Args>(args))...) + 1; // Extra space for '\0'
        if (size <= 0) {
            throw std::runtime_error("Error during formatting.");
        }
        formatBuffer.resize(std::max(formatBuffer.size(), size));
        snprintf(formatBuffer.data(), size, format.c_str(), internal::extractCString(std::forward<Args>(args))...);
        return std::string(formatBuffer.begin(), formatBuffer.begin() + size - 1); // We don't want the '\0' inside
    }
}

#endif //UNITTESTBOT_STRINGFORMAT_H
