#ifndef UNITTESTBOT_KLEEASSUMEPARAMVISITOR_H
#define UNITTESTBOT_KLEEASSUMEPARAMVISITOR_H


#include "KleeAssumeVisitor.h"
#include "Tests.h"

namespace visitor {
  class KleeAssumeParamVisitor : public KleeAssumeVisitor {
  public:
    KleeAssumeParamVisitor(const types::TypesHandler *typesHandler,
                           printer::KleePrinter *printer);

    void visit(const Tests::MethodParam &param, const std::string &_outVariable);

    void visitGlobal(const Tests::MethodParam &param, const std::string &_outVariable);

  protected:
    void visitPrimitive(const types::Type &type,
                        const std::string &name,
                        const tests::AbstractValueView *view,
                        const std::string &access,
                        int depth,
                        tests::Tests::ConstructorInfo constructorInfo = {false, false}) override;

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
  };
}


#endif // UNITTESTBOT_KLEEASSUMEPARAMVISITOR_H
