/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_ABSTRACTVALUEVIEWVISITOR_H
#define UNITTESTBOT_ABSTRACTVALUEVIEWVISITOR_H

#include "Tests.h"

#include <string>

namespace visitor {
    class AbstractValueViewVisitor {
    protected:
        types::TypesHandler const * const typesHandler;
        types::PointerUsage usage;
        size_t additionalPointersCount;
        bool inUnion = false;
    public:
        explicit AbstractValueViewVisitor(types::TypesHandler const *typesHandler,
                                          types::PointerUsage usage);

        virtual ~AbstractValueViewVisitor() = default;

        virtual void visitAny(const types::Type &type,
                              const std::string &name,
                              const tests::AbstractValueView *view,
                              const std::string &access,
                              int depth);

    protected:
        virtual void visitArrayElementAfter(const types::Type &type,
                                            const std::string &name,
                                            const tests::AbstractValueView *view,
                                            const std::string &access,
                                            int depth);

        virtual void visitPointer(const types::Type &type,
                                  const std::string &name,
                                  const tests::AbstractValueView *view,
                                  const std::string &access,
                                  int depth);

        virtual void visitArray(const types::Type &type,
                                const std::string &name,
                                const tests::AbstractValueView *view,
                                const std::string &access,
                                size_t size,
                                int depth);

        virtual void visitCString(const types::Type &type,
                                  const std::string &name,
                                  const tests::AbstractValueView *view,
                                  const std::string &access,
                                  int depth);

        virtual void visitStruct(const types::Type &type,
                                 const std::string &name,
                                 const tests::AbstractValueView *view,
                                 const std::string &access,
                                 int depth);

        virtual void visitUnion(const types::Type &type,
                                const std::string &name,
                                const tests::AbstractValueView *view,
                                const std::string &access,
                                int depth);

        virtual void visitEnum(const types::Type &type,
                                    const std::string &name,
                                    const tests::AbstractValueView *view,
                                    const std::string &access,
                                    int depth);

        virtual void visitPrimitive(const types::Type &type,
                                    const std::string &name,
                                    const tests::AbstractValueView *view,
                                    const std::string &access,
                                    int depth) = 0;

        virtual void visitPointerToFunction(const types::Type &type,
                                            const std::string &name,
                                            const tests::AbstractValueView *view,
                                            const std::string &access,
                                            int depth);

        [[nodiscard]] static std::string
        getDecoratedVarName(const std::string &varName, size_t pointersCount, const std::string &access);
    };
}


#endif // UNITTESTBOT_ABSTRACTVALUEVIEWVISITOR_H
