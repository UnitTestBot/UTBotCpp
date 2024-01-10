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

        fs::path wrapperFilePath = Paths::getWrapperFilePath(projectContext, sourceFilePath);

        fs::path sourcePathRelativeToProjectDir = fs::relative(sourceFilePath, projectContext.projectPath);
        fs::path projectDirRelativeToWrapperFile =
                fs::relative(projectContext.projectPath, wrapperFilePath.parent_path());

        strInclude(Include(false, projectDirRelativeToWrapperFile / sourcePathRelativeToProjectDir));

        ss << "#pragma GCC visibility push (default)" << printer::NL;

        ss << wrapperDefinitions;

        ss << "#pragma GCC visibility pop" << printer::NL;

        FileSystemUtils::writeToFile(wrapperFilePath, ss.str());
    }
}
