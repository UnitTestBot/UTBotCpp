/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "VerboseAssertsVisitor.h"

namespace visitor {
    VerboseAssertsVisitor::VerboseAssertsVisitor(const types::TypesHandler *typesHandler,
                                                 printer::TestsPrinter *const printer,
                                                 const std::optional<LineInfo::PredicateInfo> &predicateInfo)
        : AssertsVisitor(typesHandler, printer, types::PointerUsage::RETURN, predicateInfo) {

    }

    void VerboseAssertsVisitor::visitPointer(const types::Type &type,
                                             const std::string &name,
                                             const tests::AbstractValueView *view,
                                             const std::string &access,
                                             int depth) {
        size_t size = types::TypesHandler::getElementsNumberInPointerOneDim(usage);
        printer->strForBound(IND, size) << printer->LB();
        AbstractValueViewVisitor::visitPointer(type, name, view, access, depth);
        printer->ss << printer->RB();
    }

    void VerboseAssertsVisitor::visitArray(const types::Type &type,
                                           const std::string &name,
                                           const tests::AbstractValueView *view,
                                           const std::string &access,
                                           size_t size,
                                           int depth) {
        std::vector<size_t> sizes = type.arraysSizes(usage);
        const auto &iterators = printer->printForLoopsAndReturnLoopIterators(sizes);
        const auto indexing = printer::Printer::constrMultiIndex(iterators);

        visitAny(type.baseTypeObj(), name + indexing, view, access + indexing,
                 depth + sizes.size());
        printer->closeBrackets(sizes.size());
    }
}