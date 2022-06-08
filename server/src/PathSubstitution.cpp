#include "PathSubstitution.h"

fs::path PathSubstitution::substituteLineFlag(const fs::path& path) const {
    if (path == lineFlagSubstitution.from)
        return lineFlagSubstitution.to;
    return path;
}
