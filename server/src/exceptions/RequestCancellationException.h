#ifndef UNITTESTBOT_REQUESTCANCELLATIONEXCEPTION_H
#define UNITTESTBOT_REQUESTCANCELLATIONEXCEPTION_H

#include "BaseException.h"

struct RequestCancellationException : public BaseException {
    explicit RequestCancellationException(string message) : BaseException(std::move(message)) {}
};

#endif //UNITTESTBOT_REQUESTCANCELLATIONEXCEPTION_H
