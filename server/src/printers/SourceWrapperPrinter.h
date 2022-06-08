#ifndef UNITTESTBOT_SOURCEWRAPPERPRINTER_H
#define UNITTESTBOT_SOURCEWRAPPERPRINTER_H

#include "Printer.h"
#include "ProjectContext.h"
#include "building/BuildDatabase.h"

namespace printer {
    class SourceWrapperPrinter : public Printer {

    public:
        explicit SourceWrapperPrinter(utbot::Language srcLanguage);

        void print(const utbot::ProjectContext &projectContext,
                      const fs::path &sourceFilePath,
                      const std::string &wrapperDefinitions);
    };
}


#endif // UNITTESTBOT_SOURCEWRAPPERPRINTER_H
