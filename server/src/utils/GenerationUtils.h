#ifndef UNITTESTBOT_GENERATIONUTILS_H
#define UNITTESTBOT_GENERATIONUTILS_H

#include "Server.h"
#include "coverage/CoverageAndResultsGenerator.h"
#include "exceptions/BaseException.h"
#include "exceptions/FileNotPresentedInCommandsException.h"
#include "exceptions/FileNotPresentedInArtifactException.h"
#include "streams/coverage/CoverageAndResultsWriter.h"
#include "streams/stubs/CLIStubsWriter.h"
#include "streams/tests/CLITestsWriter.h"
#include "stubs/StubGen.h"
#include "testgens/FileTestGen.h"
#include "testgens/FolderTestGen.h"
#include "testgens/FunctionTestGen.h"

#include <protobuf/testgen.pb.h>

using namespace testsgen;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

namespace GenerationUtils {

    std::optional<fs::path>
    findTarget(const std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> &allTargets, const std::string &name);

    std::optional<fs::path> findTarget(const BaseTestGen &baseTestGen, const std::string &name);

    template <typename TestGenT, typename RequestT>
    std::pair<std::unique_ptr<BaseTestGen>, Status> createTestsByRequest(const RequestT &request,
                                                                         ServerContext *ctx) {
        static_assert(std::is_base_of<BaseTestGen, TestGenT>::value,
                      "Type parameter must derive from BaseTestGen");
        ServerUtils::setThreadOptions(ctx, true);
        auto testsWriter = std::make_unique<CLITestsWriter>();
        auto testGen = std::make_unique<TestGenT>(request, testsWriter.get(), true);
        Status status =
            Server::TestsGenServiceImpl::ProcessBaseTestRequest(*testGen, testsWriter.get());
        if (status.error_code() == grpc::FAILED_PRECONDITION) {
            if (status.error_message() == FileNotPresentedInArtifactException::MESSAGE ||
                status.error_message() == FileNotPresentedInCommandsException::MESSAGE) {
                fs::path path = status.error_details();
                auto targetPaths = testGen->getProjectBuildDatabase()->getTargetPathsForSourceFile(path);
                LOG_S(WARNING) << "List of possible targets for current file:\n";
                for (auto const& target: targetPaths) {
                    LOG_S(WARNING) << target << "\n";
                }
                LOG_S(WARNING) << "\n";
            }
        }
        return std::make_pair(std::move(testGen), status);
    }

    template <typename TestGenT, typename RequestT>
    void createTestsAndWriteStatus(RequestT *request, ServerContext *ctx) {
        auto [_, status] = createTestsByRequest<TestGenT, RequestT>(*request, ctx);
        if (status.error_message().empty()) {
            LOG_S(INFO) << "Successfully finished tests generation.";
        } else {
            LOG_S(ERROR) << status.error_message();
        }
    }

    std::pair<std::unique_ptr<BaseTestGen>, Status>
    createProjectStubsByRequest(const ProjectRequest *request, ServerContext *ctx);

    void createProjectStubsAndWriteStatus(const ProjectRequest *request, ServerContext *ctx);

    std::pair<std::unique_ptr<CoverageAndResultsGenerator>, grpc::Status>
    generateCoverageAndResults(std::unique_ptr<CoverageAndResultsRequest> request,
                               std::unique_ptr<testsgen::SettingsContext> settingsContext,
                               bool withCoverage);

    void generateCoverageAndResultsAndWriteStatus(std::unique_ptr<CoverageAndResultsRequest> request,
                                                  std::unique_ptr<testsgen::SettingsContext> settingsContext,
                                                  bool withCoverage);

};


#endif // UNITTESTBOT_GENERATIONUTILS_H
