#include "VerboseAssertsReturnValueVisitor.h"

namespace visitor {
    VerboseAssertsReturnValueVisitor::VerboseAssertsReturnValueVisitor(
            const types::TypesHandler *typesHandler, printer::TestsPrinter *printer,
            const std::optional<LineInfo::PredicateInfo> &predicateInfo)
            : VerboseAssertsVisitor(typesHandler, printer, predicateInfo) {
    }

    void VerboseAssertsReturnValueVisitor::visit(const Tests::MethodDescription &methodDescription,
                                                 const Tests::MethodTestCase &testCase) {
        auto returnType = methodDescription.returnType.maybeReturnArray()
                              ? methodDescription.returnType.arrayClone(usage, pointerSize)
                              : methodDescription.returnType.baseTypeObj();
        if (testCase.returnValue.view->getEntryValue(nullptr) == PrinterUtils::C_NULL) {
            additionalPointersCount = methodDescription.returnType.countReturnPointers(true);
            printer->writeCodeLine(StringUtils::stringFormat("EXPECT_TRUE(%s" + PrinterUtils::EQ_OPERATOR + PrinterUtils::C_NULL + ")", PrinterUtils::ACTUAL));
            return;
        }
        additionalPointersCount = 0;
        visitAny(returnType, "", testCase.returnValue.view.get(), PrinterUtils::DEFAULT_ACCESS, 0);
    }

    void VerboseAssertsReturnValueVisitor::visitPrimitive(const types::Type &type,
                                                          const std::string &name,
                                                          const tests::AbstractValueView *view,
                                                          const std::string &access,
                                                          int depth) {
        const auto &gtestMacro = predicateMapping.at(predicate);
        std::string expectedValue = PrinterUtils::fillVarName(access, PrinterUtils::EXPECTED);
        auto signature = processExpect(type, gtestMacro, {expectedValue, getDecorateActualVarName(access)});
        signature = changeSignatureToNullCheck(signature, type, view, access);
        printer->strFunctionCall(signature.name, signature.args);
    }

    void VerboseAssertsReturnValueVisitor::visitPointer(const types::Type &type,
                                                        const std::string &name,
                                                        const tests::AbstractValueView *view,
                                                        const std::string &access,
                                                        int depth) {
        if (depth == 0) {
            VerboseAssertsVisitor::visitPointer(type, name, view, access, depth);
        } else {
            // assign NULL to pointer field
        }
    }
    void VerboseAssertsReturnValueVisitor::visitArray(const types::Type &type,
                                                      const std::string &name,
                                                      const tests::AbstractValueView *view,
                                                      const std::string &access,
                                                      size_t size,
                                                      int depth) {
        bool assignPointersToNull = type.isTypeContainsPointer() && depth > 0;
        if (!assignPointersToNull) {
            VerboseAssertsVisitor::visitArray(type, name, view, access, size, depth);
        } else {
            // assign NULL to pointer field
        }
    }
}
