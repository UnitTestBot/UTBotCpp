#ifndef UNITTESTBOT_ENVIRONMENTEXCEPTION_H
#define UNITTESTBOT_ENVIRONMENTEXCEPTION_H

#include "BaseException.h"

struct EnvironmentException : public BaseException {
    explicit EnvironmentException(std::string message) : BaseException(std::move(message)) {
    }
};

#endif // UNITTESTBOT_ENVIRONMENTEXCEPTION_H
