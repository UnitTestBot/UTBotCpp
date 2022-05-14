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
                              int depth,
                              bool isConstructor = false);

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
                                int depth,
                                bool isConstructor = false);

        virtual void visitCString(const types::Type &type,
                                  const std::string &name,
                                  const tests::AbstractValueView *view,
                                  const std::string &access,
                                  int depth);

        virtual void visitStruct(const types::Type &type,
                                 const std::string &name,
                                 const tests::AbstractValueView *view,
                                 const std::string &access,
                                 int depth,
                                 bool isConstructor = false);

        virtual void visitUnion(const types::Type &type,
                                const std::string &name,
                                const tests::AbstractValueView *view,
                                const std::string &access,
                                int depth,
                                bool isConstructor = false);

        virtual void visitEnum(const types::Type &type,
                                    const std::string &name,
                                    const tests::AbstractValueView *view,
                                    const std::string &access,
                                    int depth,
                                    bool isConstructor = false);

        virtual void visitPrimitive(const types::Type &type,
                                    const std::string &name,
                                    const tests::AbstractValueView *view,
                                    const std::string &access,
                                    int depth,
                                    bool isConstructor = false) = 0;

        virtual void visitPointerToFunction(const types::Type &type,
                                            const std::string &name,
                                            const tests::AbstractValueView *view,
                                            const std::string &access,
                                            int depth);

        [[nodiscard]] static std::string
        getDecoratedVarName(const string& varName, size_t pointersCount, const string& access);
    };
}


#endif // UNITTESTBOT_ABSTRACTVALUEVIEWVISITOR_H
