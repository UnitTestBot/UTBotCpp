/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "VerboseParameterVisitor.h"

#include "printers/TestsPrinter.h"

namespace visitor {
    VerboseParameterVisitor::VerboseParameterVisitor(const types::TypesHandler *typesHandler,
                                                     printer::TestsPrinter *printer,
                                                     bool needDeclaration,
                                                     types::PointerUsage usage)
        : AbstractValueViewVisitor(typesHandler, usage), printer(printer),
          needDeclaration(needDeclaration) {
    }

    VerboseParameterVisitor::~VerboseParameterVisitor() = default;

    static thread_local std::optional<uint64_t> parameterAlignment;

    void VerboseParameterVisitor::visit(const types::Type &type,
                                        const std::string &name,
                                        const tests::AbstractValueView *view,
                                        const std::optional<uint64_t> alignment) {

        parameterAlignment = alignment;
        visitAny(type, name, view, PrinterUtils::DEFAULT_ACCESS, 0);
    }

    void VerboseParameterVisitor::visitPointer(const types::Type &type,
                                               const std::string &name,
                                               const tests::AbstractValueView *view,
                                               const std::string &access,
                                               int depth) {
        if (depth == 0) {
            if (needDeclaration) {
                printer->strDeclareArrayVar(type, name, usage, view->getEntryValue(), parameterAlignment);
            } else {
                static const std::string bufferSuffix = "_buffer";
                std::string buffer = name + bufferSuffix;
                printer->strDeclareArrayVar(type, buffer, usage, view->getEntryValue());
                size_t size = types::TypesHandler::getElementsNumberInPointerOneDim(usage);
                std::string callocCall = StringUtils::stringFormat("(%s) calloc(%zu, sizeof(%s))",
                                                              type.usedType(), size, type.baseType());
                printer->strAssignVar(name, callocCall);
                printer->strMemcpy(name, buffer, false);
            }
        } else {
            printer->strAssignVar(name, view->getEntryValue());
        }
    }

    void VerboseParameterVisitor::visitArray(const types::Type &type,
                                             const std::string &name,
                                             const tests::AbstractValueView *view,
                                             const std::string &access,
                                             size_t size,
                                             int depth,
                                             tests::Tests::ConstructorInfo constructorInfo) {
        if (needDeclaration) {
            printer->strDeclareArrayVar(type, name, usage, view->getEntryValue(), parameterAlignment);
        } else {
            std::string bufferSuffix = "_buffer";
            std::string buffer = name + bufferSuffix;
            printer->strDeclareArrayVar(type, buffer, usage, view->getEntryValue(), parameterAlignment);
            printer->strMemcpy(name, buffer, false);
        }

    }

    void VerboseParameterVisitor::visitCString(const types::Type &type,
                                               const std::string &name,
                                               const tests::AbstractValueView *view,
                                               const std::string &access,
                                               int depth) {
        std::string bufferSuffix = "_buffer";
        std::string buffer = name + bufferSuffix;
        printer->strDeclareArrayVar(type, buffer, usage, view->getEntryValue(), parameterAlignment);
        if (needDeclaration) {
            printer->strDeclareVar(type.usedType(), name, buffer);
        } else {
            printer->strAssignVar(name, buffer);
        }
    }

    void VerboseParameterVisitor::visitStruct(const types::Type &type,
                                              const std::string &name,
                                              const tests::AbstractValueView *view,
                                              const std::string &access,
                                              int depth,
                                              tests::Tests::ConstructorInfo constructorInfo) {
        if (depth == 0) {
            auto value = view->getEntryValue();
            if (needDeclaration) {
                printer->strDeclareVar(type.usedType(), name, value, parameterAlignment);
            } else {
                printer->strAssignVar(name, value);
            }
        } else {
            AbstractValueViewVisitor::visitStruct(type, name, view, access, depth);
        }
    }
    void VerboseParameterVisitor::visitUnion(const types::Type &type,
                                             const std::string &name,
                                             const tests::AbstractValueView *view,
                                             const std::string &access,
                                             int depth,
                                             tests::Tests::ConstructorInfo constructorInfo) {
        auto value = view->getEntryValue();
        if (depth == 0) {
            if (needDeclaration) {
                printer->strDeclareVar(type.usedType(), name, value, parameterAlignment);
            } else {
                printer->strAssignVar(name, value);
            }
        } else {
            printer->strAssignVar(name, value);
        }
    }
    void VerboseParameterVisitor::visitPrimitive(const types::Type &type,
                                                 const std::string &name,
                                                 const tests::AbstractValueView *view,
                                                 const std::string &access,
                                                 int depth,
                                                 tests::Tests::ConstructorInfo constructorInfo) {
        const auto typeName = types::TypesHandler::cBoolToCpp(type.usedType());
        auto value = view->getEntryValue();
        if (depth == 0) {
            if (needDeclaration) {
                printer->strDeclareVar(typeName, name, value, parameterAlignment);
            } else {
                printer->strAssignVar(name, value);
            }
        } else {
            printer->strAssignVar(name, value);
        }
    }
}
