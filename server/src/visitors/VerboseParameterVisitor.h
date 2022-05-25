/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_VERBOSEPARAMETERVISITOR_H
#define UNITTESTBOT_VERBOSEPARAMETERVISITOR_H


#include "AbstractValueViewVisitor.h"
#include "Tests.h"

namespace printer {
    class TestsPrinter;
}

namespace visitor {
    class VerboseParameterVisitor : AbstractValueViewVisitor {
        printer::TestsPrinter *const printer;
        bool needDeclaration;

    public:
        VerboseParameterVisitor(const types::TypesHandler *typesHandler,
                                printer::TestsPrinter *printer,
                                bool needDeclaration,
                                types::PointerUsage usage);

        ~VerboseParameterVisitor() override;

        void visit(const types::Type &type,
                   const std::string &name,
                   const tests::AbstractValueView *view,
                   const std::optional<uint64_t> alignment);

    private:
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
                        int depth,
                        tests::Tests::ConstructorInfo constructorInfo = {false, false}) override;

        void visitCString(const types::Type &type,
                          const std::string &name,
                          const tests::AbstractValueView *view,
                          const std::string &access,
                          int depth) override;

        void visitStruct(const types::Type &type,
                         const std::string &name,
                         const tests::AbstractValueView *view,
                         const std::string &access,
                         int depth,
                         tests::Tests::ConstructorInfo constructorInfo = {false, false}) override;
      
        void visitUnion(const types::Type &type,
                        const std::string &name,
                        const tests::AbstractValueView *view,
                        const std::string &access,
                        int depth,
                        tests::Tests::ConstructorInfo constructorInfo = {false, false}) override;
      
        void visitPrimitive(const types::Type &type,
                            const std::string &name,
                            const tests::AbstractValueView *view,
                            const std::string &access,
                            int depth,
                            tests::Tests::ConstructorInfo constructorInfo = {false, false}) override;

    };
}


#endif // UNITTESTBOT_VERBOSEPARAMETERVISITOR_H
