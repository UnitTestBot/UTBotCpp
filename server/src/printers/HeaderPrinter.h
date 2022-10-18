#ifndef UNITTESTBOT_HEADERPRINTER_H
#define UNITTESTBOT_HEADERPRINTER_H

#include "Printer.h"
#include "ProjectContext.h"
#include "types/Types.h"

namespace printer {
    class HeaderPrinter : Printer {
    public:
        HeaderPrinter(utbot::Language srcLanguage) : Printer(srcLanguage) {
        }

        utbot::Language getLanguage() const override;

        void print(const fs::path &testHeaderFilePath,
                   const fs::path &sourceFilePath,
                   std::string &headerCode);

        void processHeader(const Include &relatedHeader);
    };
}


#endif // UNITTESTBOT_HEADERPRINTER_H
