#include "CoverageAndResultsWriter.h"

CoverageAndResultsWriter::CoverageAndResultsWriter(
    grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *writer)
    : ServerWriter(writer) {
}
