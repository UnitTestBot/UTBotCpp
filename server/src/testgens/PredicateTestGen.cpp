#include "PredicateTestGen.h"

#include "utils/ExecUtils.h"

PredicateTestGen::PredicateTestGen(const testsgen::PredicateRequest &request,
                                   ProgressWriter *progressWriter,
                                   bool testMode)
    : LineTestGen(request.linerequest(), progressWriter, testMode),
      type(request.predicateinfo().type()), predicate(request.predicateinfo().predicate()),
      returnValue(request.predicateinfo().returnvalue()) {
}

std::string PredicateTestGen::toString() {
    std::stringstream s;
    s << LineTestGen::toString() << "\n\ttype: " << testsgen::ValidationType_Name(type)
      << "\n\tpredicate: " << predicate << "\n\treturnValue: " << returnValue << "\n";
    return s.str();
}

bool PredicateTestGen::needToAddPathFlag() {
    return false;
}
