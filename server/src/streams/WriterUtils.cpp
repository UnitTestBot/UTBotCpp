#include "WriterUtils.h"

void writeSourceLine(testsgen::SourceLine *sourceLineGrpc, Coverage::FileCoverage::SourceLine sourceLine) {
    sourceLineGrpc->set_line(sourceLine.line);
}
