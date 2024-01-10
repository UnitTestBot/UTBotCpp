#include "HeaderPrinter.h"

#include "utils/FileSystemUtils.h"

#include <fstream>

namespace printer {
    void HeaderPrinter::print(const fs::path &testHeaderFilePath,
                              const fs::path &sourceFilePath,
                              std::string &headerCode) {
        processHeader(Include(true, "cstring"));
        processHeader(Include(true, "unistd.h"));
        processHeader(Include(true, "stdio.h"));
        ss << printer::NL;
        ss << PrinterUtils::redirectStdin << printer::NL;
        ss << PrinterUtils::writeToFile << printer::NL;
        ss << PrinterUtils::fromBytes << printer::NL;
        ss << PrinterUtils::constCast;
        headerCode += ss.str();
        FileSystemUtils::writeToFile(testHeaderFilePath, headerCode);
    }

    void HeaderPrinter::processHeader(const Include &relatedHeader) {
        if (relatedHeader.is_angled) {
            strIncludeSystem(relatedHeader.path);
        } else {
            strInclude(relatedHeader.path);
        }
    }

    utbot::Language HeaderPrinter::getLanguage() const {
        return utbot::Language::CXX;
    }
}
