#include "FunctionPointerForStubsVisitor.h"

#include "printers/Printer.h"

#include <unordered_set>
#include "utils/StubsUtils.h"

namespace visitor {
    FunctionPointerForStubsVisitor::FunctionPointerForStubsVisitor(
        const types::TypesHandler *typesHandler)
        : AbstractValueViewVisitor(typesHandler, types::PointerUsage::RETURN) {
    }

    static thread_local printer::Printer printer{};
    static thread_local std::unordered_set<uint64_t> used;

    std::string FunctionPointerForStubsVisitor::visit(const tests::Tests &tests) {
        printer.resetStream();
        used.clear();
        for (const auto &[methodName, testMethod] : tests.methods) {
            for (auto const &param : testMethod.params) {
                visitAny(param.type, param.name, nullptr, PrinterUtils::DEFAULT_ACCESS, 0);
            }
            for (auto const &param : testMethod.globalParams) {
                visitAny(param.type, param.name, nullptr, PrinterUtils::DEFAULT_ACCESS, 0);
            }
            visitAny(testMethod.returnType, "", nullptr, PrinterUtils::DEFAULT_ACCESS, 0);
        }
        return printer.ss.str();
    }

    void FunctionPointerForStubsVisitor::visitStruct(const types::Type &type,
                                                     const std::string &name,
                                                     const tests::AbstractValueView *view,
                                                     const std::string &access,
                                                     int depth) {
        auto [_, inserted] = used.insert(type.getId());
        if (!inserted) {
            return;
        }
        auto structInfo = typesHandler->getStructInfo(type);
        for (const auto &[name, field] : structInfo.functionFields) {
            auto stubName = StubsUtils::getFunctionPointerAsStructFieldStubName(structInfo.name, name, true);
            printer.writeStubForParam(typesHandler, field, structInfo.name, stubName, false, true);
        }
        for (auto &field : structInfo.fields) {
            if (!types::TypesHandler::isPointerToFunction(field.type) &&
                !types::TypesHandler::isArrayOfPointersToFunction(field.type)) {
                visitAny(field.type, name, nullptr, access, depth + 1);
            }
        }
    }

    void FunctionPointerForStubsVisitor::visitPointer(const types::Type &type,
                                                      const std::string &name,
                                                      const tests::AbstractValueView *view,
                                                      const std::string &access,
                                                      int depth) {
        AbstractValueViewVisitor::visitPointer(type, name, view, access, depth);
    }

    void FunctionPointerForStubsVisitor::visitArray(const types::Type &type,
                                                    const std::string &name,
                                                    const tests::AbstractValueView *view,
                                                    const std::string &access,
                                                    size_t size,
                                                    int depth) {
        AbstractValueViewVisitor::visitAny(type.baseTypeObj(), name, view, access,
                                           depth + type.getDimension());
    }

    void FunctionPointerForStubsVisitor::visitPrimitive(const types::Type &type,
                                                        const std::string &name,
                                                        const tests::AbstractValueView *view,
                                                        const std::string &access,
                                                        int depth) {
        // need to be implemented
    }
}
