#ifndef UNITTESTBOT_GRPCUTILS_H
#define UNITTESTBOT_GRPCUTILS_H

#include "ProjectContext.h"
#include "ProjectTarget.h"
#include "utils/path/FileSystemPath.h"

#include <protobuf/testgen.grpc.pb.h>

#include <experimental/type_traits>
#include <memory>
#include <utility>

using namespace ::testsgen;

namespace GrpcUtils {
    extern const std::string UTBOT_AUTO_TARGET_PATH;

    std::unique_ptr<testsgen::ProjectContext>
    createProjectContext(const std::string &projectName,
                         const fs::path &projectPath,
                         const fs::path &testDirPath,
                         const fs::path &buildDirRelativePath);

    std::unique_ptr<testsgen::SettingsContext>
    createSettingsContext(bool generateForStaticFunctions,
                          bool verbose,
                          int32_t timeoutPerFunction,
                          int32_t timeoutPerTest,
                          bool useDeterministicSearcher,
                          bool useStubs,
                          ErrorMode errorMode);

    std::unique_ptr<testsgen::SnippetRequest>
    createSnippetRequest(std::unique_ptr<testsgen::ProjectContext> projectContext,
                         std::unique_ptr<testsgen::SettingsContext> settingsContext,
                         const fs::path &filePath);

    std::unique_ptr<testsgen::ProjectRequest>
    createProjectRequest(std::unique_ptr<testsgen::ProjectContext> projectContext,
                         std::unique_ptr<testsgen::SettingsContext> settingsContext,
                         const std::vector<fs::path> &sourcePaths,
                         std::optional<std::string> target = std::nullopt);


    std::unique_ptr<testsgen::FolderRequest>
    createFolderRequest(std::unique_ptr<testsgen::ProjectRequest> projectRequest,
                        fs::path const &folderPath);

    std::unique_ptr<testsgen::FileRequest>
    createFileRequest(std::unique_ptr<testsgen::ProjectRequest> projectRequest,
                      const fs::path &filePath);

    std::unique_ptr<testsgen::SourceInfo> createSourceInfo(const fs::path &filePath, uint32_t line);

    std::unique_ptr<testsgen::PredicateInfo>
    createPredicateInfo(const std::string &predicate,
                        const std::string &value,
                        const testsgen::ValidationType type);

    std::unique_ptr<testsgen::LineRequest>
    createLineRequest(std::unique_ptr<testsgen::ProjectRequest> projectRequest,
                      std::unique_ptr<testsgen::SourceInfo> lineInfo);

    std::unique_ptr<testsgen::FunctionRequest>
    createFunctionRequest(std::unique_ptr<testsgen::LineRequest> lineRequest);

    std::unique_ptr<testsgen::PredicateRequest>
    createPredicateRequest(std::unique_ptr<testsgen::LineRequest> lineRequest,
                           std::unique_ptr<testsgen::PredicateInfo> predicateInfo);

    std::unique_ptr<testsgen::AssertionRequest>
    createAssertionRequest(std::unique_ptr<testsgen::LineRequest> lineRequest);

    std::unique_ptr<testsgen::PredicateRequest>
    createPredicateRequest(std::unique_ptr<testsgen::LineRequest> lineRequest,
                           std::unique_ptr<testsgen::PredicateInfo> predicateInfo);

    std::unique_ptr<testsgen::ClassRequest>
    createClassRequest(std::unique_ptr<testsgen::LineRequest> lineRequest);

    std::unique_ptr<testsgen::CoverageAndResultsRequest>
    createCoverageAndResultRequest(std::unique_ptr<testsgen::ProjectContext> projectContext,
                                   std::unique_ptr<testsgen::TestFilter> testFilter);

    std::unique_ptr<testsgen::TestFilter> createTestFilterForProject();

    std::unique_ptr<testsgen::TestFilter> createTestFilterForFile(const fs::path &testFilePath);

    std::unique_ptr<testsgen::TestFilter> createTestFilterForTest(const fs::path &testFilePath,
                                                                  std::string testSuite,
                                                                  std::string testName);

    std::unique_ptr<testsgen::Progress>
    createProgress(std::optional<std::string> message, double percent, bool completed);

    void initProjectTarget(testsgen::ProjectTarget &projectTarget,
                           utbot::ProjectContext const &projectContext,
                           fs::path const &output);

    testsgen::ProjectTarget createAutoTarget();

    template <typename Request>
    using has_projectrequest = decltype(std::declval<Request>().projectrequest());

    template <typename Request>
    using has_linerequest = decltype(std::declval<Request>().linerequest());

    template <typename Request> bool synchronizeCode(Request const &request) {
        if constexpr (std::is_same_v<Request, testsgen::SnippetRequest>) {
            return true;
        } else if constexpr (std::is_same_v<Request, testsgen::ProjectRequest>) {
            return request.synchronizecode();
        } else if constexpr (std::experimental::is_detected_v<has_projectrequest, Request>) {
            return synchronizeCode(request.projectrequest());
        } else if constexpr (std::experimental::is_detected_v<has_linerequest, Request>) {
            return synchronizeCode(request.linerequest());
        }
    }
}


#endif // UNITTESTBOT_GRPCUTILS_H
