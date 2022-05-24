/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_KLEEASSUMERETURNVALUEVISITOR_H
#define UNITTESTBOT_KLEEASSUMERETURNVALUEVISITOR_H

#include "KleeAssumeVisitor.h"
#include "Tests.h"

namespace visitor {
    class KleeAssumeReturnValueVisitor : public KleeAssumeVisitor {
    public:
        explicit KleeAssumeReturnValueVisitor(const types::TypesHandler *typesHandler,
                                              printer::KleePrinter *printer);

        void visit(const tests::Tests::MethodDescription &methodDescription,
                   const std::optional<LineInfo::PredicateInfo> &predicateInfo);

    protected:
        void visitPointer(const types::Type &type,
                          const std::string &name,
                          const tests::AbstractValueView *view,
                          const std::string &access,
                          int depth) override;

        void visitPrimitive(const types::Type &type,
                            const std::string &name,
                            const tests::AbstractValueView *view,
                            const std::string &access,
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

        void visitArray(const types::Type &type,
                        const std::string &name,
                        const tests::AbstractValueView *view,
                        const std::string &access,
                        size_t size,
                        int depth) override;

    private:
        bool returnTypeIsArray = false;

        [[nodiscard]] std::string getDecorateTmpVarName(const std::string &access = PrinterUtils::DEFAULT_ACCESS) const;

        void checkNotNullBefore();

        void checkNotNullAfter();

        void kleeAssumeWithNullCheck(const std::string &assumption, bool useBasicAssumeIfNotPointer = true);

        static types::Type getActualTmpVarType(const types::Type &type);
    };
}


#endif // UNITTESTBOT_KLEEASSUMERETURNVALUEVISITOR_H
