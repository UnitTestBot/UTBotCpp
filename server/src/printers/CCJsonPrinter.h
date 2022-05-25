/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_CCJSONPRINTER_H
#define UNITTESTBOT_CCJSONPRINTER_H

#include "KleeGenerator.h"
#include "Printer.h"

#include <string>

namespace printer {
    class CCJsonPrinter : Printer {
    public:
        static const std::string CLANG_COMPILER;
        static const std::string DEFAULT_BUILD_FLAGS;

        static void createDummyBuildDB(const CollectionUtils::FileSet &filePaths,
                                       const fs::path &tmpDirPath);

        static void createCDb(const std::vector<utbot::CompileCommand> &compileCommands,
                              const fs::path &tmpDirPath);

    private:
        [[nodiscard]] static json getUnit(const std::vector<std::string> &command,
                                          const fs::path &directory,
                                          const std::vector<fs::path> &sourceFiles,
                                          bool forLinkJson);
    };
}


#endif // UNITTESTBOT_CCJSONPRINTER_H
