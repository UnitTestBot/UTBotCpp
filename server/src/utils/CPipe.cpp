#include "CPipe.h"

#include "LogUtils.h"

#include "loguru.h"

#include <stdexcept>
#include <unistd.h>

CPipe::CPipe() {
    if (pipe(fd)) {
        std::string message = "Failed to create pipe";
        LOG_S(ERROR) << message;
        throw std::runtime_error(message);
    }
}

int CPipe::readFd() const {
    return fd[0];
}

int CPipe::writeFd() const {
    return fd[1];
}

void CPipe::close() {
    int closeReadStatus = ::close(fd[0]);
    int closeWriteStatus = ::close(fd[1]);
    if (closeReadStatus < 0) {
        LOG_S(ERROR) << "Calling close on read pipe failed" << LogUtils::errnoMessage();
    }
    if (closeWriteStatus < 0) {
        LOG_S(ERROR) << "Calling close on write pipe failed" << LogUtils::errnoMessage();
    }
}
