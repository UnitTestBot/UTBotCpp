/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_KLEEASSUMEVISITOR_H
#define UNITTESTBOT_KLEEASSUMEVISITOR_H

#include "AbstractValueViewVisitor.h"
#include "Tests.h"
#include "printers/KleePrinter.h"

namespace visitor {
    class KleeAssumeVisitor : public AbstractValueViewVisitor {
    protected:
        printer::KleePrinter *const printer;

    public:
        KleeAssumeVisitor(const types::TypesHandler *typesHandler, printer::KleePrinter *printer);

    protected:
        void visitPointer(const types::Type &type,
                          const std::string &name,
                          const tests::AbstractValueView *view,
                          const std::string &access,
                          int depth) override;

        void kleeAssume(std::string const &assumption);
    };
}


#endif // UNITTESTBOT_KLEEASSUMEVISITOR_H
