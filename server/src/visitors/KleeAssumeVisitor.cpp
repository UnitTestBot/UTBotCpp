#include "KleeAssumeVisitor.h"

#include "KleeAssumeReturnValueVisitor.h"
#include "utils/PrinterUtils.h"

namespace visitor {
    KleeAssumeVisitor::KleeAssumeVisitor(const types::TypesHandler *typesHandler,
                                         printer::KleePrinter *printer)
        : AbstractValueViewVisitor(typesHandler/*, types::PointerUsage::RETURN*/), printer(printer) {
    }

    void KleeAssumeVisitor::visitPointer(const types::Type &type,
                                         const std::string &name,
                                         const tests::AbstractValueView *view,
                                         const std::string &access,
                                         int depth) {
        if (depth == 0) {
            AbstractValueViewVisitor::visitPointer(type, name, view, access, depth);
        } else {
            std::string assumption = PrinterUtils::getEqualString(name, PrinterUtils::C_NULL);
            kleeAssume(assumption);
        }
    }

    void KleeAssumeVisitor::kleeAssume(const std::string &assumption) {
        printer->strFunctionCall(PrinterUtils::KLEE_ASSUME, { assumption });
    }
}
