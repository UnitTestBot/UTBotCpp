#ifndef UNITTESTBOT_COVERAGEJSONWRITER_H
#define UNITTESTBOT_COVERAGEJSONWRITER_H

#include "streams/ServerWriter.h"
#include "coverage/Coverage.h"
#include "exceptions/ExecutionProcessException.h"

#include <grpcpp/grpcpp.h>
#include "json.hpp"
#include <protobuf/testgen.grpc.pb.h>


class CoverageAndResultsWriter : public utbot::ServerWriter<testsgen::CoverageAndResultsResponse> {
public:

    explicit CoverageAndResultsWriter(grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *writer);

    virtual void writeResponse(const utbot::ProjectContext &projectContext,
                               const Coverage::TestResultMap &testsResultMap,
                               const Coverage::CoverageMap &coverageMap,
                               const nlohmann::json &totals,
                               std::optional<std::string> errorMessage) = 0;
};


#endif // UNITTESTBOT_COVERAGEJSONWRITER_H
