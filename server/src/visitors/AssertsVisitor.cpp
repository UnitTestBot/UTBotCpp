#include "AssertsVisitor.h"

#include "VerboseAssertsVisitor.h"

#include <utility>

namespace visitor {
    /* Such mapping is used as we generate tests
     * in form EXPECT_(expected, actual) and
     * therefore we need to flip the operator
     */
    const std::unordered_map<std::string, std::string> AssertsVisitor::predicateMapping = {
        { "", "EQ" },  { "==", "EQ" }, { "!=", "NE" }, { ">", "LT" },
        { "<", "GT" }, { ">=", "LE" }, { "<=", "GE" }
    };

    AssertsVisitor::AssertsVisitor(const types::TypesHandler *typesHandler,
                                   printer::TestsPrinter *printer,
                                   types::PointerUsage _usage,
                                   const std::optional<LineInfo::PredicateInfo> &predicateInfo)
        : AbstractValueViewVisitor(typesHandler, _usage), printer(printer) {
       if (predicateInfo.has_value()) {
            predicate = predicateInfo->predicate;
            if (predicateInfo->type == testsgen::STRING) {
                usage = types::PointerUsage::KNOWN_SIZE;
                pointerSize = predicateInfo->returnValue.size();
            }
        }
    }

    AssertsVisitor::FunctionSignature AssertsVisitor::processExpect(
        const types::Type &type, const std::string &gtestMacro, std::vector<std::string> &&args) {
        if (types::TypesHandler::isFloatingPointType(type) && gtestMacro == PrinterUtils::EQ) {
            const types::TypeName &typeName = type.baseType();
            if (typeName == "float") {
                return VerboseAssertsVisitor::FunctionSignature{ PrinterUtils::EXPECT_FLOAT_EQ, std::move(args) };
            }
            if (typeName == "double" || typeName == "long double") {
                return VerboseAssertsVisitor::FunctionSignature{ PrinterUtils::EXPECT_DOUBLE_EQ, std::move(args) };
            }
        }
        return VerboseAssertsVisitor::FunctionSignature{ PrinterUtils::EXPECT_ + gtestMacro, std::move(args) };
    }

    std::string AssertsVisitor::getDecorateActualVarName(const std::string &access) {
        return AbstractValueViewVisitor::getDecoratedVarName(PrinterUtils::ACTUAL, additionalPointersCount,
                                                             access);
    }

    AssertsVisitor::FunctionSignature AssertsVisitor::changeSignatureToNullCheck(const FunctionSignature& signature,
                                                                                 const types::Type& type,
                                                                                 const tests::AbstractValueView *view,
                                                                                 const std::string &access) {
        if (additionalPointersCount > 0 && view->getEntryValue(nullptr) == PrinterUtils::C_NULL) {
            return processExpect(type, "TRUE", {
                    PrinterUtils::fillVarName(access, PrinterUtils::ACTUAL) + " == " + PrinterUtils::C_NULL });
        }
        return signature;
    }
}
