#include "FunctionTestGen.h"

#include "utils/ExecUtils.h"

FunctionTestGen::FunctionTestGen(const testsgen::FunctionRequest &request,
                                 ProgressWriter *progressWriter,
                                 bool testMode)
    : LineTestGen(request.linerequest(), progressWriter, testMode) {
}

std::string FunctionTestGen::toString() {
    return LineTestGen::toString();
}

bool FunctionTestGen::needToAddPathFlag() {
    return false;
}
