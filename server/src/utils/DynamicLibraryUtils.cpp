#include "DynamicLibraryUtils.h"

namespace DynamicLibraryUtils {

    fs::path getLibraryDirectoryFlag(const fs::path &directory) {
        return StringUtils::stringFormat("%s%s", libraryDirOption, directory);
    }

    fs::path getRpathDirectoryFlag(const fs::path &directory) {
        return StringUtils::stringFormat("%s,%s=%s", libraryDirOptionWl, rpathFlag, directory);
    }

    std::optional<fs::path> getLibraryAbsolutePath(const std::string &argument,
                                                   const fs::path &directory) {
        if (StringUtils::startsWith(argument, libraryDirOption)) {
            fs::path relativePath = argument.substr(libraryDirOption.length());
            fs::path absolutePath = Paths::getCCJsonFileFullPath(relativePath, directory);
            return absolutePath;
        }
        return std::nullopt;
    }
};
