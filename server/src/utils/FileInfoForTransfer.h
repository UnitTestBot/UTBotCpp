//

#ifndef UTBOTCPP_FILEINFOFORTRANSFER_H
#define UTBOTCPP_FILEINFOFORTRANSFER_H

#include <string>
#include "utils/path/FileSystemPath.h"

struct FileInfoForTransfer {
    fs::path filePath;
    std::string code;
    FileInfoForTransfer() = default;
    FileInfoForTransfer(fs::path filePath, std::string code) : filePath(std::move(filePath)), code(std::move(code)) {}
};
#endif //UTBOTCPP_FILEINFOFORTRANSFER_H
