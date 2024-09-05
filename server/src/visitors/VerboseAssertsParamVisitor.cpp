#include "VerboseAssertsParamVisitor.h"
#include "utils/KleeUtils.h"
#include "utils/PrinterUtils.h"

namespace visitor {
    VerboseAssertsParamVisitor::VerboseAssertsParamVisitor(
            const types::TypesHandler *typesHandler,
            printer::TestsPrinter *printer)
            : VerboseAssertsVisitor(typesHandler, printer, {}) {
//        usage = types::PointerUsage::PARAMETER;
    }

    static thread_local std::string expectedVariable;

    void VerboseAssertsParamVisitor::visit(const Tests::MethodParam &param, const std::string &name,
                                           const tests::AbstractValueView *view) {
        expectedVariable = PrinterUtils::getExpectedVarName(name);
        std::string paramName = param.dataVariableName();
        types::Type paramType = param.type;
        visitAny(paramType, paramName, view, PrinterUtils::DEFAULT_ACCESS, 0);
        expectedVariable = {};
    }

    //TODO: make normal solution
    void VerboseAssertsParamVisitor::visitTemp(const Tests::MethodParam &param, const std::string &name,
                                               const tests::AbstractValueView *view) {
        expectedVariable = name;
        std::string paramName = param.dataVariableName();
        types::Type paramType = param.type;
        visitAny(paramType, paramName, view, PrinterUtils::DEFAULT_ACCESS, 0);
        expectedVariable = {};
    }

    void VerboseAssertsParamVisitor::visitGlobal(const Tests::MethodParam &param, const std::string &name,
                                                 const tests::AbstractValueView *view) {
        expectedVariable = PrinterUtils::getExpectedVarName(name);
        visitAny(param.type, name, view, PrinterUtils::DEFAULT_ACCESS, 0);
        expectedVariable = {};
    }

    void VerboseAssertsParamVisitor::visitPointer(const types::Type &type,
                                                  const std::string &name,
                                                  const tests::AbstractValueView *view,
                                                  const std::string &access,
                                                  int depth) {
//        auto signature = processExpect(type, PrinterUtils::EQ,
//                                       {PrinterUtils::fillVarName(access, expectedVariable), name});
//        printer->strFunctionCall(signature.name, signature.args);
    }

    void VerboseAssertsParamVisitor::visitArray(const types::Type &type,
                                                const std::string &name,
                                                const tests::AbstractValueView *view,
                                                const std::string &access,
                                                int depth) {
        if (depth == 0) {
            if (type.isObjectPointer()) {
                return visitPointer(type, name, view, access, depth);
            }
        }
        bool assignPointersToNull = type.isTypeContainsPointer() && depth > 0;
        if (!assignPointersToNull) {
            VerboseAssertsVisitor::visitArray(type, name, view, access, depth);
        }
    }

    void VerboseAssertsParamVisitor::visitPrimitive(const types::Type &type,
                                                    const std::string &name,
                                                    const tests::AbstractValueView *view,
                                                    const std::string &access,
                                                    int depth) {
        auto signature = processExpect(type, PrinterUtils::EQ,
                                       {PrinterUtils::fillVarName(access, expectedVariable), name});
        printer->strFunctionCall(signature.name, signature.args);
    }
}
