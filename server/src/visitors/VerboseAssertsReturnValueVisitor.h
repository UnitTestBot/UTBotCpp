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
                          const std::string &name,
                          const tests::AbstractValueView *view,
                          const std::string &access,
                          int depth) override;

        void visitPrimitive(const types::Type &type,
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


#endif // UNITTESTBOT_VERBOSEASSERTSRETURNVALUEVISITOR_H
