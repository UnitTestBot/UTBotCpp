#ifndef UNITTESTBOT_NOSUCHTYPEEXCEPTION_H
#define UNITTESTBOT_NOSUCHTYPEEXCEPTION_H

#include "BaseException.h"

struct NoSuchTypeException : public BaseException {
    explicit NoSuchTypeException(std::string message) : BaseException(message) {}
};

#endif //UNITTESTBOT_NOSUCHTYPEEXCEPTION_H
