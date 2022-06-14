#ifndef UNITTESTBOT_COVERAGEGENERATIONEXCEPTION_H
#define UNITTESTBOT_COVERAGEGENERATIONEXCEPTION_H

#include "BaseException.h"

struct CoverageGenerationException : public BaseException {
    explicit CoverageGenerationException(std::string message) : BaseException(std::move(message)) {
    }

    explicit CoverageGenerationException(const char *message) : BaseException(message) {
    }
};

#endif // UNITTESTBOT_COVERAGEGENERATIONEXCEPTION_H
