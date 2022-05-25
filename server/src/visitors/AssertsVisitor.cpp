/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

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
        bool changePredicate = types::TypesHandler::isFloatingPointType(type) && (gtestMacro == PrinterUtils::EQ);
        std::string targetMacro = gtestMacro;
        if (changePredicate) {
            targetMacro = "NEAR";
            args.emplace_back(PrinterUtils::ABS_ERROR);
        }
        return VerboseAssertsVisitor::FunctionSignature{ PrinterUtils::EXPECT_ + targetMacro, std::move(args) };
    }

    std::string AssertsVisitor::getDecorateActualVarName(const std::string &access) {
        return AbstractValueViewVisitor::getDecoratedVarName(PrinterUtils::ACTUAL, additionalPointersCount,
                                                             access);
    }

    AssertsVisitor::FunctionSignature AssertsVisitor::changeSignatureToNullCheck(const FunctionSignature& signature,
                                                                                 const types::Type& type,
                                                                                 const tests::AbstractValueView *view,
                                                                                 const std::string &access) {
        if (additionalPointersCount > 0 && view->getEntryValue() == PrinterUtils::C_NULL) {
            return processExpect(type, "TRUE", {
                    PrinterUtils::fillVarName(access, PrinterUtils::ACTUAL) + " == " + PrinterUtils::C_NULL });
        }
        return signature;
    }
}