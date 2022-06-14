#include "SourceWrapperPrinter.h"

#include "Paths.h"
#include "utils/FileSystemUtils.h"

namespace printer {
    SourceWrapperPrinter::SourceWrapperPrinter(utbot::Language srcLanguage) : Printer(srcLanguage) {
    }

    void SourceWrapperPrinter::print(const utbot::ProjectContext &projectContext,
                                     const fs::path &sourceFilePath,
                                     const std::string &wrapperDefinitions) {
        if (Paths::isCXXFile(sourceFilePath))
            return;
        writeCopyrightHeader();

        strDefine("main", "main__");
        strInclude(Include(false, sourceFilePath));

        ss << wrapperDefinitions;

        fs::path wrapperFilePath = Paths::getWrapperFilePath(projectContext, sourceFilePath);
        FileSystemUtils::writeToFile(wrapperFilePath, ss.str());
    }
}
