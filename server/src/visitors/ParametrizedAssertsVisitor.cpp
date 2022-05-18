/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ParametrizedAssertsVisitor.h"
#include "Tests.h"

namespace visitor {
    ParametrizedAssertsVisitor::ParametrizedAssertsVisitor(
        const types::TypesHandler *typesHandler,
        printer::TestsPrinter *printer,
        const std::optional<LineInfo::PredicateInfo> &predicateInfo,
        bool isError)
        : AssertsVisitor(typesHandler, printer, types::PointerUsage::RETURN, predicateInfo),
          isError(isError) {
    }

    static thread_local std::string functionCall;

    void ParametrizedAssertsVisitor::visit(const Tests::MethodDescription &methodDescription,
                                           const Tests::MethodTestCase &testCase) {
        auto returnType = methodDescription.returnType.maybeReturnArray()
                              ? methodDescription.returnType.arrayClone(usage, pointerSize)
                              : methodDescription.returnType;
        functionCall = printer->constrVisitorFunctionCall(methodDescription, testCase, false);
        if (testCase.returnValueView->getEntryValue() == PrinterUtils::C_NULL) {
            additionalPointersCount = methodDescription.returnType.countReturnPointers(true);
            printer->writeCodeLine(StringUtils::stringFormat(
                "EXPECT_TRUE(%s)",
                PrinterUtils::getEqualString(functionCall, PrinterUtils::C_NULL)));
            return;
        } else {
            additionalPointersCount = 0;
        }
        visitAny(returnType, "", testCase.returnValueView.get(), PrinterUtils::DEFAULT_ACCESS, 0,
                 methodDescription.constructorInfo);
        functionCall = {};
        additionalPointersCount = 0;
    }

    void ParametrizedAssertsVisitor::visitArray(const types::Type &type,
                                                const string &name,
                                                const tests::AbstractValueView *view,
                                                const string &access,
                                                size_t size,
                                                int depth,
                                                tests::Tests::ConstructorInfo constructorInfo) {
        if (depth == 0) {
            if (type.isArray()) {
                if (isError) {
                    printer->writeCodeLine(functionCall);
                    return;
                } else {
                    printer->strDeclareVar(printer::Printer::getConstQualifier(type) +
                                               type.usedType(),
                                           PrinterUtils::ACTUAL, functionCall, std::nullopt, true,
                                           additionalPointersCount);
                    printer->strDeclareArrayVar(
                        type, PrinterUtils::fillVarName(access, PrinterUtils::EXPECTED), usage,
                        view->getEntryValue(), std::nullopt, true);
                }
            } else {
                return AbstractValueViewVisitor::visitAny(type.baseTypeObj(), name, view, access,
                                                          depth);
            }
        }

        bool assignPointersToNull = type.isTypeContainsPointer() && depth > 0;
        if (!assignPointersToNull) {
            std::vector<size_t> sizes = type.arraysSizes(usage);
            const auto &iterators = printer->printForLoopsAndReturnLoopIterators(sizes);
            const auto indexing = printer::Printer::constrMultiIndex(iterators);
            visitAny(type.baseTypeObj(), name + indexing, view, access + indexing,
                     depth + sizes.size());
            printer->closeBrackets(sizes.size());
        }
    }

    void ParametrizedAssertsVisitor::visitStruct(const types::Type &type,
                                                 const string &name,
                                                 const tests::AbstractValueView *view,
                                                 const string &access,
                                                 int depth,
                                                 tests::Tests::ConstructorInfo constructorInfo) {
        if (depth == 0) {
            std::optional<std::string> initValue = functionCall;
            if (constructorInfo.isMoveConstructor) {
                initValue = "std::move(" + functionCall +  ")";
            }
            printer->strDeclareVar(printer::Printer::getConstQualifier(type) + type.usedType(),
                                   PrinterUtils::ACTUAL, initValue, std::nullopt, true,
                                   additionalPointersCount);
            if (!constructorInfo.isConstructor) {
                printer->strDeclareVar(type.typeName(),
                                       PrinterUtils::fillVarName(access, PrinterUtils::EXPECTED),
                                       view->getEntryValue());
            }
        }
        AbstractValueViewVisitor::visitStruct(type, name, view, access, depth, constructorInfo);
    }

    void ParametrizedAssertsVisitor::visitUnion(const types::Type &type,
                                                const string &name,
                                                const tests::AbstractValueView *view,
                                                const string &access,
                                                int depth,
                                                tests::Tests::ConstructorInfo constructorInfo) {
        if (depth == 0) {
            printer->strDeclareVar(printer::Printer::getConstQualifier(type) + type.usedType(),
                                   PrinterUtils::ACTUAL, functionCall, std::nullopt, true,
                                   additionalPointersCount);
            if (!constructorInfo.isConstructor) {
                printer->strDeclareVar(type.typeName(), PrinterUtils::EXPECTED,
                                       view->getEntryValue());
            }
        }
        AbstractValueViewVisitor::visitUnion(type, name, view, access, depth, constructorInfo);
    }

    void ParametrizedAssertsVisitor::visitPrimitive(const types::Type &type,
                                                    const string &name,
                                                    const tests::AbstractValueView *view,
                                                    const string &access,
                                                    int depth,
                                                    tests::Tests::ConstructorInfo constructorInfo) {
        if (depth == 0) {
            if (types::TypesHandler::isVoid(type) || isError) {
                printer->writeCodeLine(functionCall);
            } else {
                printer->strDeclareVar(printer::Printer::getConstQualifier(type) + type.usedType(),
                                       PrinterUtils::ACTUAL, functionCall, std::nullopt, true,
                                       additionalPointersCount);
                const auto &gtestMacro = predicateMapping.at(predicate);
                auto signature = processExpect(
                    type, gtestMacro, { view->getEntryValue(), getDecorateActualVarName(access) });
                signature = changeSignatureToNullCheck(signature, type, view, access);
                printer->strFunctionCall(signature.name, signature.args, SCNL, std::nullopt, true,
                                         0, std::nullopt, inUnion);
            }
        } else {
            if (isError) {
                return;
            }
            const auto &gtestMacro = predicateMapping.at(predicate);
            std::string expectedValue;
            if (!constructorInfo.isConstructor) {
                expectedValue = PrinterUtils::fillVarName(access, PrinterUtils::EXPECTED);
            } else {
                expectedValue = view->getEntryValue();
            }
            auto signature = processExpect(type, gtestMacro,
                                           { getDecorateActualVarName(access), expectedValue });
            signature = changeSignatureToNullCheck(signature, type, view, access);
            printer->strFunctionCall(signature.name, signature.args, SCNL, std::nullopt, true, 0,
                                     std::nullopt, inUnion);
        }
    }

    void ParametrizedAssertsVisitor::visitPointer(const types::Type &type,
                                                  const string &name,
                                                  const tests::AbstractValueView *view,
                                                  const string &access,
                                                  int depth) {
        if (depth == 0) {
            AbstractValueViewVisitor::visitAny(type.baseTypeObj(), name, view, access, depth);
        } else {
            // assign NULL to pointer field
        }
    }

    void ParametrizedAssertsVisitor::visitPointerToFunction(const types::Type &type,
                                                            const string &name,
                                                            const tests::AbstractValueView *view,
                                                            const string &access,
                                                            int depth) {
        printer->writeCodeLine(functionCall);
    }
}