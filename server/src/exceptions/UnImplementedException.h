#ifndef UNITTESTBOT_UNIMPLEMENTEDEXCEPTION_H
#define UNITTESTBOT_UNIMPLEMENTEDEXCEPTION_H

#include "BaseException.h"

struct UnImplementedException : public BaseException {
    explicit UnImplementedException(std::string message) : BaseException(message) {
    }
};

#endif // UNITTESTBOT_NOSUCHTYPEEXCEPTION_H
