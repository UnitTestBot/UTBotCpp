/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_VERBOSEASSERTSRETURNVALUEVISITOR_H
#define UNITTESTBOT_VERBOSEASSERTSRETURNVALUEVISITOR_H

#include "VerboseAssertsVisitor.h"

namespace visitor {
    class VerboseAssertsReturnValueVisitor : public VerboseAssertsVisitor {
    public:
        VerboseAssertsReturnValueVisitor(const types::TypesHandler *typesHandler,
                                         printer::TestsPrinter *printer,
                                         const std::optional<LineInfo::PredicateInfo>& predicateInfo);

        void visit(const Tests::MethodDescription& methodDescription,
                   const Tests::MethodTestCase &testCase);

    protected:
        void visitPointer(const types::Type &type,
                          const string &name,
                          const tests::AbstractValueView *view,
                          const string &access,
                          int depth) override;
        void visitPrimitive(const types::Type &type,
                            const string &name,
                            const tests::AbstractValueView *view,
                            const string &access,
                            int depth,
                            bool isConstructor = false) override;
        void visitArray(const types::Type &type,
                        const string &name,
                        const tests::AbstractValueView *view,
                        const string &access,
                        size_t size,
                        int depth,
                        bool isConstructor = false) override;
    };
}


#endif // UNITTESTBOT_VERBOSEASSERTSRETURNVALUEVISITOR_H
