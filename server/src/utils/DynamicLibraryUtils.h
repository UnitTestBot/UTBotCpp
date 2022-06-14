#ifndef UNITTESTBOT_DYNAMICLIBRARYUTILS_H
#define UNITTESTBOT_DYNAMICLIBRARYUTILS_H

#include "Paths.h"
#include "StringUtils.h"

#include "utils/path/FileSystemPath.h"
#include <optional>
#include <string>

namespace DynamicLibraryUtils {
    inline static const std::string libraryDirOption = "-L";
    inline static const std::string libraryDirOptionWl = "-Wl";
    inline static const std::string rpathFlag = "-rpath";
    inline static const std::string linkFlag = "-l";

    fs::path getLibraryDirectoryFlag(fs::path const &directory);

    fs::path getRpathDirectoryFlag(fs::path const &directory);

    std::optional<fs::path> getLibraryAbsolutePath(std::string const &argument,
                                                   fs::path const &directory);
};


#endif // UNITTESTBOT_DYNAMICLIBRARYUTILS_H
