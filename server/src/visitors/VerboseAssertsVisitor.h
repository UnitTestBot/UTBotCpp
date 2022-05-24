/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_VERBOSEASSERTSVISITOR_H
#define UNITTESTBOT_VERBOSEASSERTSVISITOR_H

#include "AssertsVisitor.h"

#include <unordered_map>

namespace visitor {
    class VerboseAssertsVisitor : public AssertsVisitor {

    protected:

        explicit VerboseAssertsVisitor(const types::TypesHandler *typesHandler,
                                       printer::TestsPrinter *printer,
                                       const std::optional<LineInfo::PredicateInfo> &predicateInfo);

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
    };
}


#endif // UNITTESTBOT_VERBOSEASSERTSVISITOR_H
