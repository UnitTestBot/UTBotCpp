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
        fs::path wrapperFilePath = Paths::getWrapperFilePath(projectContext, sourceFilePath);
        auto content = getFinalContent(projectContext, sourceFilePath, wrapperDefinitions);
        FileSystemUtils::writeToFile(wrapperFilePath, content);
    }

    std::string SourceWrapperPrinter::getFinalContent(const utbot::ProjectContext &projectContext,
                                     const fs::path &sourceFilePath,
                                     const std::string &wrapperDefinitions) {
        if (Paths::isCXXFile(sourceFilePath))
            throw std::invalid_argument(StringUtils::stringFormat("Tried to get wrapper for cpp file: %s", sourceFilePath));

        writeCopyrightHeader();
        strDefine("main", "main__");

        fs::path wrapperFilePath = Paths::getWrapperFilePath(projectContext, sourceFilePath);
        fs::path sourcePathRelativeToProjectDir = fs::relative(sourceFilePath, projectContext.projectPath);
        fs::path projectDirRelativeToWrapperFile =
                fs::relative(projectContext.projectPath, wrapperFilePath.parent_path());

        strInclude(Include(false, projectDirRelativeToWrapperFile / sourcePathRelativeToProjectDir));

        ss << wrapperDefinitions;

        return ss.str();
    }
}
