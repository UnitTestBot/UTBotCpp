#ifndef UNITTESTBOT_STUBSPRINTER_H
#define UNITTESTBOT_STUBSPRINTER_H

#include "Printer.h"
#include "ProjectContext.h"
#include "stubs/Stubs.h"
#include "types/Types.h"
#include "utils/StubsUtils.h"

namespace printer {
    class StubsPrinter : public Printer {
    public:
        StubsPrinter(utbot::Language srcLanguage);

        Stubs genStubFile(const Tests &tests,
                          const types::TypesHandler &typesHandler,
                          const utbot::ProjectContext &projectContext);
    };
}


#endif // UNITTESTBOT_STUBSPRINTER_H
