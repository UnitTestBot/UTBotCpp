/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "GenerationUtils.h"

#include <Server.h>
#include <streams/coverage/CLICoverageAndResultsWriter.h>
#include <streams/stubs/CLIStubsWriter.h>

std::pair<std::unique_ptr<BaseTestGen>, Status>
GenerationUtils::createProjectStubsByRequest(const ProjectRequest *request, ServerContext *ctx) {
    ServerUtils::setThreadOptions(ctx, true);
    auto stubsWriter = std::make_unique<CLIStubsWriter>();
    auto testGen = std::make_unique<ProjectTestGen>(*request, stubsWriter.get(), true);
    Status status =
        Server::TestsGenServiceImpl::ProcessProjectStubsRequest(testGen.get(), stubsWriter.get());
    return std::make_pair(std::move(testGen), status);
}

void GenerationUtils::createProjectStubsAndWriteStatus(const ProjectRequest *request,
                                                       ServerContext *ctx) {
    auto [_, status] = createProjectStubsByRequest(request, ctx);
    if (status.error_message().empty()) {
        LOG_S(INFO) << "Successfully generated stubs.";
    } else {
        LOG_S(ERROR) << status.error_message();
    }
}
std::pair<std::unique_ptr<CoverageAndResultsGenerator>, grpc::Status>
GenerationUtils::generateCoverageAndResults(
    std::unique_ptr<CoverageAndResultsRequest> request,
    std::unique_ptr<testsgen::SettingsContext> settingsContext,
    bool withCoverage) {
    auto coverageAndResultsWriter = std::make_unique<CLICoverageAndResultsWriter>(
        fs::path(request->projectcontext().projectpath()) /
        request->projectcontext().resultsdirrelativepath());
    auto coverageGenerator = std::make_unique<CoverageAndResultsGenerator>(
        request.get(), coverageAndResultsWriter.get());
    auto status = coverageGenerator->generate(withCoverage, *settingsContext);
    return std::make_pair(std::move(coverageGenerator), status);
}

void GenerationUtils::generateCoverageAndResultsAndWriteStatus(
    std::unique_ptr<CoverageAndResultsRequest> request,
    std::unique_ptr<testsgen::SettingsContext> settingsContext,
    bool withCoverage) {
    auto [_, status] =
        generateCoverageAndResults(std::move(request), std::move(settingsContext), withCoverage);
    if (status.error_message().empty()) {
        LOG_S(INFO) << "Successfully ran tests.";
    } else {
        LOG_S(ERROR) << status.error_message();
    }
}

std::optional<fs::path> GenerationUtils::findTarget(const BaseTestGen &baseTestGen,
                                                    const std::string &name) {
    if (name.empty()) {
        LOG_S(INFO) << "Target was not chosen. Using UTBot: Auto target instead.";
        return GrpcUtils::UTBOT_AUTO_TARGET_PATH;
    }
    auto allTargets = baseTestGen.buildDatabase->getAllTargets();
    auto candidates = CollectionUtils::filterOut(allTargets, [&name](auto const &target) {
        fs::path output = target->getOutput();
        return !(output == name || output.stem() == name || output.stem() == "lib" + name);
    });
    if (candidates.empty()) {
        LOG_S(WARNING) << "Couldn't find appropriate target. List of available targets:\n";
        for (const auto &target : allTargets) {
            LOG_S(WARNING) << target->getOutput() << "\n";
        }
        LOG_S(WARNING) << "\n";
        return std::nullopt;
    } else if (candidates.size() == 1) {
        return candidates[0]->getOutput();
    } else {
        LOG_S(WARNING) << "There are multiple candidates for given target. "
                          "Please, specify full path of your preferred target. "
                          "List of candidates:\n";
        for (const auto &candidate : candidates) {
            LOG_S(WARNING) << candidate->getOutput() << "\n";
        }
        LOG_S(WARNING) << "\n";
        return std::nullopt;
    }
}
