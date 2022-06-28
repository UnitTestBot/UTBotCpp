#include "GrpcUtils.h"

namespace GrpcUtils {
    static const std::string UTBOT_AUTO_TARGET = "UTBot: auto";
    const std::string UTBOT_AUTO_TARGET_PATH = fs::path("/utbot/auto/target/path");
    static inline const std::string UTBOT_AUTO_TARGET_DESCRIPTION = "Finds any target that contains the code under testing";

    std::unique_ptr<testsgen::SnippetRequest>
    createSnippetRequest(std::unique_ptr<testsgen::ProjectContext> projectContext,
                         std::unique_ptr<testsgen::SettingsContext> settingsContext,
                         const fs::path &filePath) {
        auto result = std::make_unique<testsgen::SnippetRequest>();
        result->set_allocated_projectcontext(projectContext.release());
        result->set_allocated_settingscontext(settingsContext.release());
        result->set_filepath(filePath);
        return result;
    }

    std::unique_ptr<testsgen::ProjectContext>
    createProjectContext(const std::string &projectName,
                         const fs::path &projectPath,
                         const fs::path &testDirPath,
                         const fs::path &buildDirRelativePath) {
        auto result = std::make_unique<testsgen::ProjectContext>();
        result->set_projectname(projectName);
        result->set_projectpath(projectPath);
        result->set_testdirpath(testDirPath);
        result->set_builddirrelativepath(buildDirRelativePath);
        return result;
    }

    std::unique_ptr<testsgen::SettingsContext>
    createSettingsContext(bool generateForStaticFunctions,
                          bool verbose,
                          int32_t timeoutPerFunction,
                          int32_t timeoutPerTest,
                          bool useDeterministicSearcher,
                          bool useStubs,
                          bool genSarif) {
        auto result = std::make_unique<testsgen::SettingsContext>();
        result->set_generateforstaticfunctions(generateForStaticFunctions);
        result->set_verbose(verbose);
        result->set_timeoutperfunction(timeoutPerFunction);
        result->set_timeoutpertest(timeoutPerTest);
        result->set_usedeterministicsearcher(useDeterministicSearcher);
        result->set_usestubs(useStubs);
        result->set_gensarif(genSarif);
        return result;
    }

    std::unique_ptr<testsgen::ProjectRequest>
    createProjectRequest(std::unique_ptr<testsgen::ProjectContext> projectContext,
                         std::unique_ptr<testsgen::SettingsContext> settingsContext,
                         const std::vector<fs::path> &sourcePaths,
                         std::optional<std::string> target) {
        auto result = std::make_unique<testsgen::ProjectRequest>();
        result->set_allocated_projectcontext(projectContext.release());
        result->set_allocated_settingscontext(settingsContext.release());
        for (const auto &path : sourcePaths) {
            result->add_sourcepaths(path);
        }
        if (target.has_value()) {
            result->set_targetpath(target.value());
        }
        return result;
    }

    std::unique_ptr<testsgen::FolderRequest>
    createFolderRequest(std::unique_ptr<testsgen::ProjectRequest> projectRequest,
                        const fs::path &folderPath) {
        auto result = std::make_unique<testsgen::FolderRequest>();
        result->set_allocated_projectrequest(projectRequest.release());
        result->set_folderpath(folderPath);
        return result;
    }

    std::unique_ptr<testsgen::FileRequest>
    createFileRequest(std::unique_ptr<testsgen::ProjectRequest> projectRequest,
                      const fs::path &filePath) {
        auto result = std::make_unique<testsgen::FileRequest>();
        result->set_allocated_projectrequest(projectRequest.release());
        result->set_filepath(filePath);
        return result;
    }

    std::unique_ptr<testsgen::LineRequest>
    createLineRequest(std::unique_ptr<testsgen::ProjectRequest> projectRequest,
                      std::unique_ptr<testsgen::SourceInfo> lineInfo) {
        auto result = std::make_unique<testsgen::LineRequest>();
        result->set_allocated_projectrequest(projectRequest.release());
        result->set_allocated_sourceinfo(lineInfo.release());
        return result;
    }

    std::unique_ptr<testsgen::FunctionRequest>
    createFunctionRequest(std::unique_ptr<testsgen::LineRequest> lineRequest) {
        auto result = std::make_unique<testsgen::FunctionRequest>();
        result->set_allocated_linerequest(lineRequest.release());
        return result;
    }

    std::unique_ptr<testsgen::AssertionRequest>
    createAssertionRequest(std::unique_ptr<testsgen::LineRequest> lineRequest) {
        auto result = std::make_unique<testsgen::AssertionRequest>();
        result->set_allocated_linerequest(lineRequest.release());
        return result;
    }

    std::unique_ptr<testsgen::PredicateRequest>
    createPredicateRequest(std::unique_ptr<testsgen::LineRequest> lineRequest,
                           std::unique_ptr<testsgen::PredicateInfo> predicateInfo) {
        auto result = std::make_unique<testsgen::PredicateRequest>();
        result->set_allocated_linerequest(lineRequest.release());
        result->set_allocated_predicateinfo(predicateInfo.release());
        return result;
    }

    std::unique_ptr<testsgen::ClassRequest>
    createClassRequest(std::unique_ptr<testsgen::LineRequest> lineRequest) {
        auto result = std::make_unique<testsgen::ClassRequest>();
        result->set_allocated_linerequest(lineRequest.release());
        return result;
    }

    std::unique_ptr<testsgen::CoverageAndResultsRequest>
    createCoverageAndResultRequest(std::unique_ptr<testsgen::ProjectContext> projectContext,
                                   std::unique_ptr<testsgen::TestFilter> testFilter) {
        auto result = std::make_unique<testsgen::CoverageAndResultsRequest>();
        result->set_allocated_projectcontext(projectContext.release());
        result->set_allocated_testfilter(testFilter.release());
        result->set_coverage(true);
        return result;
    }

    std::unique_ptr<testsgen::TestFilter> createTestFilterForProject() {
        return std::make_unique<testsgen::TestFilter>();
    }

    std::unique_ptr<testsgen::TestFilter> createTestFilterForFile(const fs::path &testFilePath) {
        auto testFilter = std::make_unique<testsgen::TestFilter>();
        testFilter->set_testfilepath(testFilePath);
        return testFilter;
    }

    std::unique_ptr<testsgen::TestFilter> createTestFilterForTest(const fs::path &testFilePath,
                                                                  std::string testSuite,
                                                                  std::string testName) {
        auto testFilter = std::make_unique<testsgen::TestFilter>();
        testFilter->set_testfilepath(testFilePath);
        testFilter->set_testsuite(std::move(testSuite));
        testFilter->set_testname(std::move(testName));
        return testFilter;
    }

    std::unique_ptr<testsgen::SourceInfo> createSourceInfo(const fs::path &filePath,
                                                           uint32_t line) {
        auto result = std::make_unique<testsgen::SourceInfo>();
        result->set_filepath(filePath);
        result->set_line(line);
        return result;
    }

    std::unique_ptr<testsgen::PredicateInfo> createPredicateInfo(const std::string &predicate,
                                                                 const std::string &value,
                                                                 const testsgen::ValidationType type) {
        auto result = std::make_unique<testsgen::PredicateInfo>();
        result->set_type(type);
        result->set_predicate(predicate);
        result->set_returnvalue(value);
        return result;
    }

    std::unique_ptr<testsgen::Progress>
    createProgress(std::optional<std::string> message, double percent, bool completed) {
        auto result = std::make_unique<testsgen::Progress>();
        result->set_completed(completed);
        if (message.has_value()) {
            result->set_message(message.value());
        }
        result->set_percent(percent);
        return result;
    }

    void initProjectTarget(testsgen::ProjectTarget &projectTarget,
                           const utbot::ProjectContext &projectContext,
                           const fs::path &output) {
        projectTarget.set_name(output.filename());
        projectTarget.set_path(output);
        fs::path description =
            fs::relative(output, projectContext.projectPath / projectContext.buildDirRelativePath);
        projectTarget.set_description(description);
    }

    testsgen::ProjectTarget createAutoTarget() {
        testsgen::ProjectTarget result;
        result.set_name(UTBOT_AUTO_TARGET);
        result.set_path(UTBOT_AUTO_TARGET_PATH);
        result.set_description(UTBOT_AUTO_TARGET_DESCRIPTION);
        return result;
    }
}
