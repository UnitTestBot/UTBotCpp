#ifndef UNITTESTBOT_BUILDRESULT_H
#define UNITTESTBOT_BUILDRESULT_H

#include "utils/path/FileSystemPath.h"

struct BuildResult {
    fs::path output;
    enum class Type { NONE, NO_STUBS, ANY_STUBS, ALL_STUBS } type;
};

BuildResult::Type operator|(BuildResult::Type a, BuildResult::Type b);

BuildResult::Type operator|=(BuildResult::Type &a, BuildResult::Type b);

#endif // UNITTESTBOT_BUILDRESULT_H
