#include "AssertionTestGen.h"

#include "utils/ExecUtils.h"

AssertionTestGen::AssertionTestGen(const testsgen::AssertionRequest &request,
                                   ProgressWriter *progressWriter,
                                   bool testMode)
    : LineTestGen(request.linerequest(), progressWriter, testMode) {
}

std::string AssertionTestGen::toString() {
    return LineTestGen::toString();
}
