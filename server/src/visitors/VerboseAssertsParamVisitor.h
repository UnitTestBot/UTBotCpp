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

    void visitPrimitive(const types::Type &type,
                        const std::string &name,
                        const tests::AbstractValueView *view,
                        const std::string &access,
                        int depth) override;
  };
}


#endif //UNITTESTBOT_VERBOSEASSERTSPARAMVISITOR_H
