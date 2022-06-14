#ifndef UNITTESTBOT_WRITERUTILS_H
#define UNITTESTBOT_WRITERUTILS_H

#include "coverage/Coverage.h"
#include "stubs/Stubs.h"

#include <protobuf/testgen.grpc.pb.h>

void writeSourceLine(testsgen::SourceLine *sourceLineGrpc, Coverage::FileCoverage::SourceLine sourceLine);

#endif //UNITTESTBOT_WRITERUTILS_H
