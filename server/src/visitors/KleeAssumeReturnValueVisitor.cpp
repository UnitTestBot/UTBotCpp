/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "KleeAssumeReturnValueVisitor.h"

#include "utils/KleeUtils.h"
#include "utils/PrinterUtils.h"

namespace visitor {
    KleeAssumeReturnValueVisitor::KleeAssumeReturnValueVisitor(
        const types::TypesHandler *typesHandler, printer::KleePrinter *printer)
        : KleeAssumeVisitor(typesHandler, printer) {
    }

    static thread_local std::string functionCall;

    void KleeAssumeReturnValueVisitor::visit(
        const tests::Tests::MethodDescription &methodDescription,
        const std::optional<LineInfo::PredicateInfo> &predicateInfo) {
        functionCall = printer->constrFunctionCall(methodDescription, 0, "", false);
        additionalPointersCount = methodDescription.returnType.countReturnPointers();
        auto returnType = methodDescription.returnType.baseTypeObj();
        printer->strDeclareVar(getActualTmpVarType(returnType).baseType(),
                           KleeUtils::TEMP_VARIABLE_NAME, functionCall,
                           std::nullopt, true, additionalPointersCount);
        checkNotNullBefore();
        if (predicateInfo.has_value()) {
            string assumption;
            if (predicateInfo->type != testsgen::STRING) {
                assumption = PrinterUtils::getEqualString(KleeUtils::RESULT_VARIABLE_NAME, KleeUtils::TEMP_VARIABLE_NAME);
                kleeAssumeWithNullCheck(assumption);
                assumption = StringUtils::stringFormat(
                        "%s %s %s", KleeUtils::RESULT_VARIABLE_NAME, predicateInfo->predicate,
                PrinterUtils::wrapUserValue(predicateInfo->type, predicateInfo->returnValue));
                kleeAssume(assumption);
            } else {
                for (int i = 0; i < predicateInfo->returnValue.size(); i++) {
                    assumption = StringUtils::stringFormat("%s[%d] == \'%c\'", KleeUtils::RESULT_VARIABLE_NAME, i, predicateInfo->returnValue[i]);
                    kleeAssume(assumption);
                }
                for (int i = 0; i < predicateInfo->returnValue.size(); i++) {
                    assumption = StringUtils::stringFormat("%s[%d] %s %s[%d]", KleeUtils::RESULT_VARIABLE_NAME,
                                                                  i,
                                                                  predicateInfo->predicate,
                                                                  KleeUtils::TEMP_VARIABLE_NAME, i);
                    kleeAssumeWithNullCheck(assumption);
                }
            }
        } else {
            if (methodDescription.returnType.maybeReturnArray()) {
                returnType = methodDescription.returnType.arrayClone(usage);
            }
            visitAny(returnType, "", nullptr, PrinterUtils::DEFAULT_ACCESS, 0);
        }
        checkNotNullAfter();
        functionCall = {};
        additionalPointersCount = 0;
    }

    void KleeAssumeReturnValueVisitor::visitPrimitive(const types::Type &type,
                                                      const string &name,
                                                      const tests::AbstractValueView *view,
                                                      const string &access,
                                                      int depth,
                                                      bool isConstructor) {
        string assumption = PrinterUtils::getEqualString(getDecorateTmpVarName(access),
                                             PrinterUtils::fillVarName(access, KleeUtils::RESULT_VARIABLE_NAME));
        kleeAssumeWithNullCheck(assumption);
    }

    void KleeAssumeReturnValueVisitor::visitStruct(const types::Type &type,
                                                   const string &name,
                                                   const tests::AbstractValueView *view,
                                                   const string &access,
                                                   int depth,
                                                   bool isConstructor) {
        if (depth == 0) {
            kleeAssumeWithNullCheck("", false);
            AbstractValueViewVisitor::visitStruct(type, KleeUtils::TEMP_VARIABLE_NAME, view, PrinterUtils::DEFAULT_ACCESS,
                                                  depth);
        } else {
            AbstractValueViewVisitor::visitStruct(type, name, view, access, depth);
        }
    }

    void KleeAssumeReturnValueVisitor::visitUnion(const types::Type &type,
                                                  const string &name,
                                                  const tests::AbstractValueView *view,
                                                  const string &access,
                                                  int depth,
                                                  bool isConstructor) {
        if (depth == 0) {
            kleeAssumeWithNullCheck("", false);
        }
        AbstractValueViewVisitor::visitUnion(type, name, view, access, depth);
    }

    void KleeAssumeReturnValueVisitor::visitPointer(const types::Type &type,
                                                    const string &name,
                                                    const tests::AbstractValueView *view,
                                                    const string &access,
                                                    int depth) {
        if (depth == 0) {
            KleeAssumeReturnValueVisitor::visitPrimitive(type, name, view, access, depth);
        } else {
            // assign NULL to pointer fields
        }
    }

    void KleeAssumeReturnValueVisitor::visitArray(const types::Type &type,
                                                  const std::string &name,
                                                  const tests::AbstractValueView *view,
                                                  const std::string &access,
                                                  size_t size,
                                                  int depth,
                                                  bool isConstructor) {
        if (depth == 0 && additionalPointersCount > 0) {
            returnTypeIsArray = true;
            additionalPointersCount--;
        }
        auto sizes = type.arraysSizes(usage);
        bool assignPointersToNull = type.isTypeContainsPointer() && depth > 0;
        if (assignPointersToNull) {
            int pointerIndex = type.indexOfFirstPointerInTypeKinds();
            sizes = std::vector<size_t>(sizes.begin(), sizes.begin() + pointerIndex);
        }
        const auto &iterators = printer->printForLoopsAndReturnLoopIterators(sizes);
        const auto indexing = printer::Printer::constrMultiIndex(iterators);

        auto baseType = type.baseTypeObj(sizes.size());
        visitAny(baseType, name + indexing, view, access + indexing, depth + sizes.size());
        printer->closeBrackets(sizes.size());
    }

   void KleeAssumeReturnValueVisitor::kleeAssumeWithNullCheck(const string& assumption, bool useBasicAssumeIfNotPointer) {
       if (!useBasicAssumeIfNotPointer && additionalPointersCount == 0) {
           return;
       }
       if (additionalPointersCount > 0) {
           auto notNullAssumptionCheck = KleeUtils::NOT_NULL_VARIABLE_NAME + " == 1";
           if (assumption.empty()) {
               kleeAssume(notNullAssumptionCheck);
           } else {
               kleeAssume(notNullAssumptionCheck + " & " + assumption);
           }
       } else {
           kleeAssume(assumption);
       }
    }

   types::Type KleeAssumeReturnValueVisitor::getActualTmpVarType(const types::Type &type) {
       if (types::TypesHandler::isVoid(type.baseTypeObj())) {
           return types::Type::minimalScalarPointerType(type.countReturnPointers());
       }
       return type;
   }

   std::string KleeAssumeReturnValueVisitor::getDecorateTmpVarName(const string &access) const {
       return AbstractValueViewVisitor::getDecoratedVarName(KleeUtils::TEMP_VARIABLE_NAME,
                                                            additionalPointersCount, access);
   }
   void KleeAssumeReturnValueVisitor::checkNotNullBefore() {
       if (additionalPointersCount > 0) {
           printer->ss << printer->TAB_N() << "if (" << KleeUtils::TEMP_VARIABLE_NAME
                       << " != " << PrinterUtils::C_NULL << ")" << printer->LB();
       }
   }

   void KleeAssumeReturnValueVisitor::checkNotNullAfter() {
       if (additionalPointersCount > 0 || returnTypeIsArray) {
           printer->ss << printer->RB();
       }
   }
}
