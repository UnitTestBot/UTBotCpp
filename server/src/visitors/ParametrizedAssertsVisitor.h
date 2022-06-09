/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PARAMETRIZEDASSERTSVISITOR_H
#define UNITTESTBOT_PARAMETRIZEDASSERTSVISITOR_H

#include "AssertsVisitor.h"

namespace visitor {
    class ParametrizedAssertsVisitor : public AssertsVisitor {
    private:
        bool isError;
    public:
        explicit ParametrizedAssertsVisitor(const types::TypesHandler *typesHandler,
                                            printer::TestsPrinter *printer,
                                            const std::optional<LineInfo::PredicateInfo> &predicateInfo,
                                            bool isError);

        void visit(const Tests::MethodDescription &methodDescription,
                   const Tests::MethodTestCase &testCase,
                   ::testsgen::ErrorMode errorMode = ::testsgen::ErrorMode::FAILING);

    protected:
        void visitPointer(const types::Type &type,
                          const std::string &name,
                          const tests::AbstractValueView *view,
                          const std::string &access,
                          int depth) override;

        void visitArray(const types::Type &type,
                        const std::string &name,
                        const tests::AbstractValueView *view,
                        const std::string &access,
                        size_t size,
                        int depth) override;

        void visitStruct(const types::Type &type,
                         const std::string &name,
                         const tests::AbstractValueView *view,
                         const std::string &access,
                         int depth) override;

        void visitUnion(const types::Type &type,
                        const std::string &name,
                        const tests::AbstractValueView *view,
                        const std::string &access,
                        int depth) override;

        void visitPrimitive(const types::Type &type,
                            const std::string &name,
                            const tests::AbstractValueView *view,
                            const std::string &access,
                            int depth) override;

        void visitPointerToFunction(const types::Type &type,
                                    const std::string &name,
                                    const tests::AbstractValueView *view,
                                    const std::string &access,
                                    int depth) override;
    };
}


#endif // UNITTESTBOT_PARAMETRIZEDASSERTSVISITOR_H
