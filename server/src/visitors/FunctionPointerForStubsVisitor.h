#ifndef UNITTESTBOT_FUNCTIONPOINTERFORSTUBSVISITOR_H
#define UNITTESTBOT_FUNCTIONPOINTERFORSTUBSVISITOR_H

#include "AbstractValueViewVisitor.h"

#include <unordered_map>

namespace visitor {
    class FunctionPointerForStubsVisitor : public AbstractValueViewVisitor {
    public:
        explicit FunctionPointerForStubsVisitor(const types::TypesHandler *typesHandler);


        std::string visit(const tests::Tests &tests);

    protected:
        void visitStruct(const types::Type &type,
                         const std::string &name,
                         const tests::AbstractValueView *view,
                         const std::string &access,
                         int depth) override;

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


#endif // UNITTESTBOT_FUNCTIONPOINTERFORSTUBSVISITOR_H
