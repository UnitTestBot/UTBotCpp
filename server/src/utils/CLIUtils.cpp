#include "CLIUtils.h"

#include "GenerationUtils.h"
#include "Paths.h"
#include "commands/Commands.h"

#include "loguru.h"

using namespace GenerationUtils;
using namespace Commands;

void CLIUtils::setOptPath(const std::string &optPath, fs::path &var) {
    if (!optPath.empty()) {
        if (Paths::isValidDir(std::string(optPath))) {
            Paths::setOptPath(var, std::string(optPath));
        } else {
            LOG_S(WARNING) << optPath << " path not valid: " << std::string(optPath);
        }
    }
}

void setStderrVerbosity(loguru::NamedVerbosity verbosity) {
    loguru::g_stderr_verbosity = verbosity;
}

void CLIUtils::setupLogger(const std::string &logPath,
                           const loguru::NamedVerbosity &verbosity,
                           bool threadView) {
    setStderrVerbosity(loguru::Verbosity_WARNING);
    loguru::g_preamble_uptime = false;
    if (!threadView) {
        loguru::g_preamble_thread = false;
    }

    CLIUtils::setOptPath(logPath, Paths::logPath);
    const fs::path symLink = Paths::getSymLinkPathToLogLatest();
    const std::string logfile_path_string = std::string(Paths::getUtbotLogAllFilePath());
    loguru::add_file(logfile_path_string.data(), loguru::Append, loguru::Verbosity_MAX);
    std::filesystem::remove(symLink.string());
    std::filesystem::create_symlink(logfile_path_string, symLink.string());

    setStderrVerbosity(verbosity);
}

std::unique_ptr<testsgen::ProjectContext>
createProjectContextByOptions(const ProjectContextOptionGroup &projectContextOptions) {
    auto projectContext =
            GrpcUtils::createProjectContext(projectContextOptions.getProjectName(),
                                            projectContextOptions.getProjectPath(),
                                            projectContextOptions.getTestDirectory(),
                                            projectContextOptions.getReportDirectory(),
                                            projectContextOptions.getBuildDirectory(),
                                            projectContextOptions.getItfRelPath());
    return projectContext;
}

std::unique_ptr<testsgen::SettingsContext>
createSettingsContextByOptions(const SettingsContextOptionGroup &settingsContextOptionGroup) {
    return GrpcUtils::createSettingsContext(
            settingsContextOptionGroup.doGenerateForStaticFunctions(),
            settingsContextOptionGroup.isVerbose(), settingsContextOptionGroup.getTimeoutPerFunction(),
            settingsContextOptionGroup.getTimeoutPerTest(),
            settingsContextOptionGroup.isDeterministicSearcherUsed(),
            settingsContextOptionGroup.withStubs(),
            settingsContextOptionGroup.getErrorMode(),
            settingsContextOptionGroup.doDifferentVariablesOfTheSameType(),
            settingsContextOptionGroup.getSkipObjectWithoutSource());
}

std::vector<fs::path> getSourcePaths(const ProjectContextOptionGroup &projectContextOptions,
                                     const std::string &sourcePathsString) {
    if (!sourcePathsString.empty()) {
        return CollectionUtils::transformTo<std::vector<fs::path>>(
            StringUtils::split(sourcePathsString, ','), [](std::string const &file) {
                return Paths::normalizedTrimmed(fs::absolute(fs::path(file)));
            });
    } else if (!projectContextOptions.getProjectPath().empty()) {
        return FileSystemUtils::recursiveDirectories(projectContextOptions.getProjectPath());
    }

    return {};
}

void CLIUtils::parse(int argc, char **argv, CLI::App &app) {

    auto mainCommands = MainCommands(app);

    auto serverCommandOptions = ServerCommandOptions(mainCommands.getServerCommand());

    auto generateCommands = GenerateCommands(mainCommands.getGenerateCommand());
    auto runCommands = RunTestsCommands(mainCommands);

    auto projectGenerateContext = ProjectContextOptionGroup(mainCommands.getGenerateCommand());
    auto settingsGenerateContext = SettingsContextOptionGroup(mainCommands.getGenerateCommand());

    auto projectRunContext = ProjectContextOptionGroup(mainCommands.getRunTestsCommand());
    auto settingsRunContext = SettingsContextOptionGroup(mainCommands.getRunTestsCommand());

    auto projectAllContext = ProjectContextOptionGroup(mainCommands.getAllCommand());
    auto settingsAllContext = SettingsContextOptionGroup(mainCommands.getAllCommand());

    auto generateCommandsOptions = GenerateCommandsOptions(generateCommands);
    auto runTestCommandsOptions = RunTestsCommandOptions(runCommands);

    auto allCommandsOptions = AllCommandOptions(mainCommands.getAllCommand());

    auto ctx = std::make_unique<ServerContext>();
    ServerUtils::setThreadOptions(ctx.get(), true);

    // PARSE RESULTS
    app.parse(argc, argv);

    CLIUtils::setupLogger(mainCommands.getLogPath(),
                          mainCommands.getVerbosity());

    if (app.got_subcommand(mainCommands.getGenerateCommand())) {
        auto sourcePaths =
            getSourcePaths(projectGenerateContext, generateCommandsOptions.getSrcPaths());
        auto projectContext = createProjectContextByOptions(projectGenerateContext);
        auto settingsContext = createSettingsContextByOptions(settingsGenerateContext);

        if (generateCommands.gotSnippetCommand()) {
            fs::path filePath =
                Paths::normalizedTrimmed(fs::absolute(generateCommandsOptions.getFilePath()));
            auto snippetRequest = GrpcUtils::createSnippetRequest(
                std::move(projectContext), std::move(settingsContext), filePath);
            createTestsAndWriteStatus<SnippetTestGen, SnippetRequest>(snippetRequest.get(),
                                                                      ctx.get());
            return;
        }

        auto target = generateCommandsOptions.getTarget();
        auto projectRequest = GrpcUtils::createProjectRequest(
            std::move(projectContext), std::move(settingsContext), sourcePaths, target);

        if (generateCommands.gotProjectCommand()) {
            createTestsAndWriteStatus<ProjectTestGen, ProjectRequest>(projectRequest.get(),
                                                                      ctx.get());

        } else if (generateCommands.gotFolderCommand()) {
            fs::path folderPath =
                Paths::normalizedTrimmed(fs::absolute(generateCommandsOptions.getFolderPath()));
            auto folderRequest =
                GrpcUtils::createFolderRequest(std::move(projectRequest), folderPath);
            createTestsAndWriteStatus<FolderTestGen, FolderRequest>(folderRequest.get(), ctx.get());

        } else if (generateCommands.gotFileCommand()) {
            fs::path filePath =
                Paths::normalizedTrimmed(fs::absolute(generateCommandsOptions.getFilePath()));
            auto fileRequest = GrpcUtils::createFileRequest(std::move(projectRequest), filePath);
            createTestsAndWriteStatus<FileTestGen, FileRequest>(fileRequest.get(), ctx.get());
        } else if (generateCommands.gotLineCommand() || generateCommands.gotFunctionCommand() ||
                   generateCommands.gotPredicateCommand() ||
                   generateCommands.gotAssertionCommand() || generateCommands.gotClassCommand()) {
            fs::path filePath =
                Paths::normalizedTrimmed(fs::absolute(generateCommandsOptions.getFilePath()));
            auto lineInfo = GrpcUtils::createSourceInfo(
                filePath, static_cast<int>(generateCommandsOptions.getLineNumber()));
            auto lineRequest =
                GrpcUtils::createLineRequest(std::move(projectRequest), std::move(lineInfo));

            if (generateCommands.gotLineCommand()) {
                createTestsAndWriteStatus<LineTestGen, LineRequest>(lineRequest.get(), ctx.get());
            } else if (generateCommands.gotFunctionCommand()) {
                auto functionRequest = GrpcUtils::createFunctionRequest(std::move(lineRequest));
                createTestsAndWriteStatus<FunctionTestGen, FunctionRequest>(functionRequest.get(),
                                                                            ctx.get());

            } else if (generateCommands.gotAssertionCommand()) {

                auto assertionRequest = GrpcUtils::createAssertionRequest(std::move(lineRequest));
                createTestsAndWriteStatus<AssertionTestGen, AssertionRequest>(
                    assertionRequest.get(), ctx.get());

            } else if (generateCommands.gotPredicateCommand()) {
                auto predicateInfo =
                    GrpcUtils::createPredicateInfo(generateCommandsOptions.getPredicate(),
                                                   generateCommandsOptions.getReturnValue(),
                                                   generateCommandsOptions.getValidationType());
                auto predicateRequest = GrpcUtils::createPredicateRequest(std::move(lineRequest),
                                                                          std::move(predicateInfo));
                createTestsAndWriteStatus<PredicateTestGen, PredicateRequest>(
                    predicateRequest.get(), ctx.get());
            } else if (mainCommands.getGenerateCommand()->got_subcommand(
                           generateCommands.getClassCommand())) {
                auto classRequest = GrpcUtils::createClassRequest(std::move(lineRequest));
                createTestsAndWriteStatus<ClassTestGen, ClassRequest>(classRequest.get(),
                                                                      ctx.get());
            }
        } else if (generateCommands.gotStubsCommand()) {
            createProjectStubsAndWriteStatus(projectRequest.get(), ctx.get());
        }

    } else if (app.got_subcommand(mainCommands.getRunTestsCommand())) {
        auto projectContext = createProjectContextByOptions(projectRunContext);
        auto settingsContext = createSettingsContextByOptions(settingsRunContext);
        if (runCommands.gotRunTestCommand()) {
            auto testFilter = GrpcUtils::createTestFilterForTest(
                runTestCommandsOptions.getFilePath(), runTestCommandsOptions.getTestSuite(),
                runTestCommandsOptions.getTestName());
            auto coverageAndResultRequest = GrpcUtils::createCoverageAndResultRequest(
                std::move(projectContext), std::move(testFilter));
            GenerationUtils::generateCoverageAndResultsAndWriteStatus(
                std::move(coverageAndResultRequest), std::move(settingsContext),
                runTestCommandsOptions.withCoverage());
        } else if (runCommands.gotRunFileCommand()) {
            auto testFilter =
                GrpcUtils::createTestFilterForFile(runTestCommandsOptions.getFilePath());
            auto coverageAndResultRequest = GrpcUtils::createCoverageAndResultRequest(
                std::move(projectContext), std::move(testFilter));
            GenerationUtils::generateCoverageAndResultsAndWriteStatus(
                std::move(coverageAndResultRequest), std::move(settingsContext),
                runTestCommandsOptions.withCoverage());
        } else if (runCommands.gotRunProjectCommand()) {
            auto testFilter = GrpcUtils::createTestFilterForProject();
            auto coverageAndResultRequest = GrpcUtils::createCoverageAndResultRequest(
                std::move(projectContext), std::move(testFilter));
            GenerationUtils::generateCoverageAndResultsAndWriteStatus(
                std::move(coverageAndResultRequest), std::move(settingsContext),
                runTestCommandsOptions.withCoverage());
        } else {
            // intentionally left blank
        }
    } else if (app.got_subcommand(mainCommands.getAllCommand())) {
        auto sourcePaths = getSourcePaths(projectAllContext, allCommandsOptions.getSrcPaths());
        auto target = allCommandsOptions.getTarget();
        auto projectRequest = GrpcUtils::createProjectRequest(
            std::move(createProjectContextByOptions(projectAllContext)),
            std::move(createSettingsContextByOptions(settingsAllContext)), sourcePaths, target);
        auto [testGen, statusTests] =
            createTestsByRequest<ProjectTestGen, ProjectRequest>(*projectRequest, ctx.get());
        if (!statusTests.error_message().empty()) {
            LOG_S(ERROR) << statusTests.error_message();
            return;
        }
        auto coverageAndResultsRequest = GrpcUtils::createCoverageAndResultRequest(
            std::move(createProjectContextByOptions(projectAllContext)),
            GrpcUtils::createTestFilterForProject());
        auto [_, statusResults] = generateCoverageAndResults(
            std::move(coverageAndResultsRequest),
            std::move(createSettingsContextByOptions(settingsAllContext)),
            allCommandsOptions.withCoverage());
        if (!statusResults.error_message().empty()) {
            LOG_S(ERROR) << statusTests.error_message();
            return;
        }

        LOG_S(INFO) << "Successfully finished.";
    } else {
        Server server;
        if (serverCommandOptions.getPort() != 0) {
            server.run(serverCommandOptions.getPort());
        } else {
            server.run();
        }
    }
}

loguru::NamedVerbosity CLIUtils::getVerbosityLevelFromName(const char *name) {
    if (strcmp(name, "debug") == 0) {
        return loguru::Verbosity_1;
    } else if (strcmp(name, "info") == 0) {
        return loguru::Verbosity_INFO;
    } else if (strcmp(name, "trace") == 0 || strcmp(name, "max") == 0) {
        return loguru::Verbosity_MAX;
    } else if (strcmp(name, "error") == 0) {
        return loguru::Verbosity_ERROR;
    }
    return loguru::Verbosity_INVALID;
}

char *CLIUtils::getCmdOption(char **begin, char **end, const std::string &option) {
    char **itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return nullptr;
}


void CLIUtils::setOptPath(int argc, char **argv, const std::string &option, fs::path &var) {
    auto optPath = getCmdOption(argv, argv + argc, option);
    if (optPath != nullptr) {
        if (Paths::isValidDir(std::string(optPath))) {
            Paths::setOptPath(var, std::string(optPath));
        } else {
            LOG_S(WARNING) << option << " path not valid: " << std::string(optPath);
        }
    }
}
void CLIUtils::setupLogger(int argc, char **argv, bool threadView) {
    setStderrVerbosity(loguru::Verbosity_WARNING);
    loguru::g_preamble_uptime = false;
    if (!threadView) {
        loguru::g_preamble_thread = false;
    }
    loguru::init(argc, argv);
    setOptPath(argc, argv, "--log", Paths::logPath);
    auto verbosityLevel = getCmdOption(argv, argv + argc, "--verbosity");
    if (verbosityLevel == nullptr) {
        return;
    }
    auto namedVerbosity = getVerbosityLevelFromName(verbosityLevel);
    if (namedVerbosity != loguru::Verbosity_INVALID) {
        setStderrVerbosity(namedVerbosity);
    }
}
