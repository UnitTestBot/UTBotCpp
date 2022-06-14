#ifndef UNITTESTBOT_ASSERTSVISITOR_H
#define UNITTESTBOT_ASSERTSVISITOR_H

#include "AbstractValueViewVisitor.h"
#include "printers/TestsPrinter.h"

#include <unordered_map>

namespace visitor {
    class AssertsVisitor : public AbstractValueViewVisitor {
    protected:
        printer::TestsPrinter *const printer;

        struct FunctionSignature {
            std::string name;
            std::vector<std::string> args;
        };

        size_t pointerSize;
        std::string predicate = "";
        static const std::unordered_map<std::string, std::string> predicateMapping;

        static FunctionSignature processExpect(const types::Type &type,
                                               const std::string &gtestMacro,
                                               std::vector<std::string> &&args);

        [[nodiscard]] std::string getDecorateActualVarName(const std::string& access);

        FunctionSignature changeSignatureToNullCheck(const FunctionSignature& signature, const types::Type& type,
                                                     const tests::AbstractValueView *view, const std::string &access);

    public:
        explicit AssertsVisitor(const types::TypesHandler *typesHandler,
                                printer::TestsPrinter *printer,
                                types::PointerUsage usage, const std::optional<LineInfo::PredicateInfo> &predicateInfo);
    };
}


#endif // UNITTESTBOT_ASSERTSVISITOR_H
