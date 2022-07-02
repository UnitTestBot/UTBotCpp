#ifndef UNITTESTBOT_TRACEUTILS_H
#define UNITTESTBOT_TRACEUTILS_H
#include "ErrorInfo.h"
#include "path/FileSystemPath.h"

ErrorInfo getErrorInfo(const fs::path &path);

#endif // UNITTESTBOT_TRACEUTILS_H
