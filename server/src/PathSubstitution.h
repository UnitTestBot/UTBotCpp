#ifndef UNITTESTBOT_PATHSUBSTITUTION_H
#define UNITTESTBOT_PATHSUBSTITUTION_H

#include "utils/CollectionUtils.h"

#include <string>
#include <unordered_map>
#include "utils/path/FileSystemPath.h"

struct PathSubstitution {
    struct LineFlagSubstitution {
        fs::path from, to;
    } lineFlagSubstitution;

    [[nodiscard]] fs::path substituteLineFlag(const fs::path& path) const;
};

#endif //UNITTESTBOT_PATHSUBSTITUTION_H
