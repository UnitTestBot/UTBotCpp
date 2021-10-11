/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "HeaderPrinter.h"

#include "utils/FileSystemUtils.h"

#include <fstream>

namespace printer {
    void HeaderPrinter::print(const fs::path &testHeaderFilePath,
                                 const fs::path &sourceFilePath,
                                 string &headerCode) {
        processHeader(Include(true, "cstring"));
        processHeader(Include(true, "unistd.h"));
        ss << NL;
        ss << PrinterUtils::redirectStdin << NL << NL;
        ss << PrinterUtils::fromBytes << NL;
        headerCode += ss.str();
        FileSystemUtils::writeToFile(testHeaderFilePath, headerCode);
    }

    void HeaderPrinter::processHeader(const Include &relatedHeader) {
        if (relatedHeader.is_angled) {
            strIncludeSystem(relatedHeader.path) << NL;
        } else {
            strInclude(relatedHeader.path) << NL;
        }
    }

    utbot::Language HeaderPrinter::getLanguage() const {
        return utbot::Language::CXX;
    }
}
