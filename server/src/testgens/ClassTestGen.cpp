#include "ClassTestGen.h"

ClassTestGen::ClassTestGen(const testsgen::ClassRequest &request,
                           ProgressWriter *progressWriter,
                           bool testMode)
    : LineTestGen(request.linerequest(), progressWriter, testMode, true) {
}

std::string ClassTestGen::toString() {
    return LineTestGen::toString();
}

bool ClassTestGen::needToAddPathFlag() {
    return false;
}
