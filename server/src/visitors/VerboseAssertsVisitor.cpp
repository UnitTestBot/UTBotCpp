#include "VerboseAssertsVisitor.h"

namespace visitor {
    VerboseAssertsVisitor::VerboseAssertsVisitor(const types::TypesHandler *typesHandler,
                                                 printer::TestsPrinter *const printer,
                                                 const std::optional<LineInfo::PredicateInfo> &predicateInfo)
        : AssertsVisitor(typesHandler, printer, predicateInfo) {

    }

    void VerboseAssertsVisitor::visitPointer(const types::Type &type,
                                             const std::string &name,
                                             const tests::AbstractValueView *view,
                                             const std::string &access,
                                             int depth) {
        printer->strForBound(printer::IND, 1) << printer->LB();
        AbstractValueViewVisitor::visitPointer(type, name, view, access, depth);
        printer->ss << printer->RB();
    }

    void VerboseAssertsVisitor::visitArray(const types::Type &type,
                                           const std::string &name,
                                           const tests::AbstractValueView *view,
                                           const std::string &access,
                                           int depth) {
        std::vector<size_t> sizes = {view->getSubViews().size()};
        const auto &iterators = printer->printForLoopsAndReturnLoopIterators(sizes);
        const auto indexing = printer::Printer::constrMultiIndex(iterators);

        //TODO change view->getSubViews().front().get()
        visitAny(type.baseTypeObj(1), name + indexing, view->getSubViews().front().get(), access + indexing,
                 depth + sizes.size());
        printer->closeBrackets(sizes.size());
    }
}
