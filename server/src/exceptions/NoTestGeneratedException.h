#ifndef UNITTESTBOT_NOTESTGENERATEDEXCEPTION_H
#define UNITTESTBOT_NOTESTGENERATEDEXCEPTION_H

#include "BaseException.h"

struct NoTestGeneratedException : public BaseException {
    explicit NoTestGeneratedException() : BaseException("") {}

    explicit NoTestGeneratedException(const std::string& message) : BaseException("") {
        this->message = defaultMessage + " " + message;
    }

    [[nodiscard]] const char *what() const noexcept override {
        if (message.empty()) {
            return defaultMessage.c_str();
        }
        return message.c_str();
    }

private:
    std::string defaultMessage = "No tests generated.";
};

#endif //UNITTESTBOT_NOTESTGENERATEDEXCEPTION_H
