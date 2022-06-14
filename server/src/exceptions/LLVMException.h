#ifndef UNITTESTBOT_LLVMEXCEPTION_H
#define UNITTESTBOT_LLVMEXCEPTION_H

#include "BaseException.h"

struct LLVMException : public BaseException {
    explicit LLVMException(std::string message) : BaseException(std::move(message)) {
    }
};

#endif //UNITTESTBOT_LLVMEXCEPTION_H
