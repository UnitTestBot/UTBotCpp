#ifndef UNITTESTBOT_CancellationException_H
#define UNITTESTBOT_CancellationException_H

#include <exception>

struct CancellationException : public std::exception {
    explicit CancellationException() = default;

    [[nodiscard]] const char *what() const noexcept override {
        return "Request has been cancelled.";
    }
};

#endif //UNITTESTBOT_CancellationException_H
