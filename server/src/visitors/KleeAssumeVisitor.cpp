/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "KleeAssumeVisitor.h"

#include "KleeAssumeReturnValueVisitor.h"
#include "utils/PrinterUtils.h"

namespace visitor {
    KleeAssumeVisitor::KleeAssumeVisitor(const types::TypesHandler *typesHandler,
                                         printer::KleePrinter *printer)
        : AbstractValueViewVisitor(typesHandler, types::PointerUsage::RETURN), printer(printer) {
    }

    void KleeAssumeVisitor::visitPointer(const types::Type &type,
                                         const string &name,
                                         const tests::AbstractValueView *view,
                                         const string &access,
                                         int depth) {
        if (depth == 0) {
            AbstractValueViewVisitor::visitPointer(type, name, view, access, depth);
        } else {
            string assumption = PrinterUtils::getEqualString(name, PrinterUtils::C_NULL);
            kleeAssume(assumption);
        }
    }

    void KleeAssumeVisitor::kleeAssume(const string &assumption) {
        printer->strFunctionCall("klee_assume", { assumption });
    }
}
