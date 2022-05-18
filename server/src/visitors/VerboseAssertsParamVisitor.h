/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_VERBOSEASSERTSPARAMVISITOR_H
#define UNITTESTBOT_VERBOSEASSERTSPARAMVISITOR_H

#include "Tests.h"
#include "VerboseAssertsVisitor.h"

namespace visitor {
  class VerboseAssertsParamVisitor : public VerboseAssertsVisitor {
  public:
    VerboseAssertsParamVisitor(const types::TypesHandler *typesHandler,
                                     printer::TestsPrinter *printer);

    void visit(const Tests::MethodParam &param, const std::string &name);

    void visitGlobal(const Tests::MethodParam &param, const std::string &name);

  protected:
    void visitPointer(const types::Type &type,
                      const string &name,
                      const tests::AbstractValueView *view,
                      const string &access,
                      int depth) override;

    void visitArray(const types::Type &type,
                    const string &name,
                    const tests::AbstractValueView *view,
                    const string &access,
                    size_t size,
                    int depth,
                    tests::Tests::ConstructorInfo constructorInfo = {false, false}) override;

    void visitPrimitive(const types::Type &type,
                        const string &name,
                        const tests::AbstractValueView *view,
                        const string &access,
                        int depth,
                        tests::Tests::ConstructorInfo constructorInfo = {false, false}) override;
  };
}


#endif //UNITTESTBOT_VERBOSEASSERTSPARAMVISITOR_H
