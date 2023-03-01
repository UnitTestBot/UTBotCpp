#include "FileSystemPath.h"
#include "utils/StringUtils.h"

namespace fs {
    path findInPATH(const path &p) {
        if (is_absolute(p)) {
            return p;
        }
        std::vector<std::string> pathENV = StringUtils::split(std::getenv("PATH"), ':');
        for (const std::string &pathFind: pathENV) {
            path fullPath = path(pathFind) / p;
            if (exists(fullPath)) {
                return fullPath;
            }
        }
        return p;
    }
}
