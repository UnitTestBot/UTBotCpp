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
        static const string CLANG_COMPILER;
        static const string DEFAULT_BUILD_FLAGS;

        static void createDummyBuildDB(const vector<fs::path> &filePaths,
                                       const fs::path &tmpDirPath);

        static void createCDb(const vector<utbot::CompileCommand> &compileCommands,
                              const fs::path &tmpDirPath);

    private:
        [[nodiscard]] static json getUnit(const vector<string> &command,
                                          const fs::path &directory,
                                          const vector<fs::path> &sourceFiles,
                                          bool forLinkJson);
    };
}


#endif // UNITTESTBOT_CCJSONPRINTER_H
