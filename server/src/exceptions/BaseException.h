#ifndef UNITTESTBOT_BASEEXCEPTION_H
#define UNITTESTBOT_BASEEXCEPTION_H

#include <string>

struct BaseException : public std::exception {
    explicit BaseException(std::string message) : message(std::move(message)) {
    }

    [[nodiscard]] virtual const char *what() const noexcept override {
        return message.c_str();
    }

    virtual ~BaseException() {
    }

protected:
    std::string message;
};

#endif // UNITTESTBOT_BASEEXCEPTION_H
