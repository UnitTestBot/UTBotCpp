#include "KleeRunner.h"

#include "Paths.h"
#include "TimeExecStatistics.h"
#include "SARIFGenerator.h"
#include "exceptions/FileNotPresentedInArtifactException.h"
#include "exceptions/FileNotPresentedInCommandsException.h"
#include "tasks/RunKleeTask.h"
#include "utils/ExecUtils.h"
#include "utils/FileSystemUtils.h"
#include "utils/KleeUtils.h"
#include "utils/LogUtils.h"
#include "TimeExecStatistics.h"
#include "utils/stats/CSVReader.h"
#include "utils/stats/TestsGenerationStats.h"
#include "utils/TraceUtils.h"

#include "loguru.h"

#include <fstream>
#include <utility>

using namespace tests;
using namespace ::testsgen;

namespace {
    void clearUnusedData(const fs::path &kleeDir) {
        fs::remove(kleeDir / "assembly.ll");
        fs::remove(kleeDir / "run.istats");
    }

    StatsUtils::KleeStats writeKleeStats(const fs::path &kleeOut) {
        ShellExecTask::ExecutionParameters kleeStatsParams("klee-stats",
                                                           {"--utbot-config", kleeOut.string(),
                                                            "--table-format=readable-csv"});
        auto[out, status, _] = ShellExecTask::runShellCommandTask(kleeStatsParams);
        if (status != 0) {
            LOG_S(ERROR) << "klee-stats call failed:" << "\n" << out;
            return {};
        }
        LOG_S(DEBUG) << "klee-stats report:" << '\n' << out;
        std::stringstream ss(out);
        return StatsUtils::KleeStats(ss);
    }
}

KleeRunner::KleeRunner(utbot::ProjectContext projectContext,
                       utbot::SettingsContext settingsContext)
    : projectContext(std::move(projectContext)), settingsContext(std::move(settingsContext)) {
}

void KleeRunner::runKlee(const std::vector<tests::TestMethod> &testMethods,
                         tests::TestsMap &testsMap,
                         const std::shared_ptr<KleeGenerator> &generator,
                         const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
                         const std::shared_ptr<LineInfo> &lineInfo,
                         TestsWriter *testsWriter,
                         bool isBatched,
                         bool interactiveMode,
                         StatsUtils::TestsGenerationStatsFileMap &generationStats) {
    LOG_SCOPE_FUNCTION(DEBUG);

    fs::path kleeOutDir = Paths::getKleeOutDir(projectContext);
    if (fs::exists(kleeOutDir)) {
        FileSystemUtils::removeAll(kleeOutDir);
    }
    fs::create_directories(kleeOutDir);
    CollectionUtils::MapFileTo<std::vector<TestMethod>> fileToMethods;
    for (const auto &method : testMethods) {
        fileToMethods[method.sourceFilePath].push_back(method);
    }

    nlohmann::json sarifResults = nlohmann::json::array();

    std::function<void(tests::Tests &tests)> prepareTests = [&](tests::Tests &tests) {
        fs::path filePath = tests.sourceFilePath;
        const auto &batch = fileToMethods[filePath];
        if (!tests.isFilePresentedInCommands) {
            if (isBatched) {
                LOG_S(WARNING) << FileNotPresentedInCommandsException::createMessage(filePath);
                return;
            } else {
                throw FileNotPresentedInCommandsException(filePath);
            }
        }
        if (!tests.isFilePresentedInArtifact) {
            if (isBatched) {
                LOG_S(WARNING) << FileNotPresentedInArtifactException::createMessage(filePath);
                return;
            } else {
                throw FileNotPresentedInArtifactException(filePath);
            }
        }
        std::vector<MethodKtests> ktests;
        ktests.reserve(batch.size());
        std::stringstream logStream;
        if (LogUtils::isMaxVerbosity()) {
            logStream << "Processing batch: ";
            for (const auto &method : batch) {
                logStream << method.methodName << ", ";
            }
            LOG_S(MAX) << logStream.str();
        }
        if (interactiveMode) {
            processBatchWithInteractive(batch, tests, ktests);
        } else {
            processBatchWithoutInteractive(batch, tests, ktests);
        }
        auto kleeStats = writeKleeStats(Paths::kleeOutDirForFilePath(projectContext, filePath));
        generator->parseKTestsToFinalCode(projectContext, tests, methodNameToReturnTypeMap, ktests,
                                          lineInfo, settingsContext.verbose, settingsContext.errorMode);
        generationStats.addFileStats(kleeStats, tests);

        sarif::sarifAddTestsToResults(projectContext, tests, sarifResults);
    };

    std::function<void()> prepareTotal = [&]() {
        testsWriter->writeReport(sarif::sarifPackResults(sarifResults),
                                 "Sarif Report was created",
                                 Paths::getUTBotReportDir(projectContext) / sarif::SARIF_FILE_NAME);
    };

    testsWriter->writeTestsWithProgress(
        testsMap,
        "Running klee",
        projectContext.testDirPath,
        std::move(prepareTests),
        std::move(prepareTotal));
}

static void processMethod(MethodKtests &ktestChunk,
                          tests::Tests &tests,
                          const fs::path &kleeOut,
                          const tests::TestMethod &method) {
    if (!fs::exists(kleeOut)) {
        return;
    }

    clearUnusedData(kleeOut);
    bool hasTimeout = false;
    bool hasError = false;
    for (auto const &entry : fs::directory_iterator(kleeOut)) {
        auto const &path = entry.path();
        if (Paths::isKtestJson(path)) {
            if (Paths::hasEarly(path)) {
                hasTimeout = true;
            } else if (Paths::hasInternalError(path)) {
                hasError = true;
            } else {
                std::unique_ptr<TestCase, decltype(&TestCase_free)> ktestData{
                    TC_fromFile(path.c_str()), TestCase_free
                };
                if (ktestData == nullptr) {
                    LOG_S(WARNING) << "Unable to open .ktestjson file";
                    continue;
                }
                const std::vector<fs::path> &errorDescriptorFiles =
                        Paths::getErrorDescriptors(path);

                UTBotKTest::Status status = errorDescriptorFiles.empty()
                                            ? UTBotKTest::Status::SUCCESS
                                            : UTBotKTest::Status::FAILED;
                std::vector<ConcretizedObject> kTestObjects(
                    ktestData->objects, ktestData->objects + ktestData->n_objects);


                std::vector<UTBotKTestObject> objects = CollectionUtils::transform(
                    kTestObjects, [](const ConcretizedObject &kTestObject) {
                        return UTBotKTestObject{ kTestObject };
                    });

                std::vector<std::string> errorDescriptors = CollectionUtils::transform(
                    errorDescriptorFiles, [](const fs::path &errorFile) {
                        std::ifstream fileWithError(errorFile.c_str(), std::ios_base::in);
                        std::string content((std::istreambuf_iterator<char>(fileWithError)),
                                            std::istreambuf_iterator<char>());

                        const std::string &errorId = errorFile.stem().extension().string();
                        if (!errorId.empty()) {
                            // skip leading dot
                            content += "\n" + sarif::ERROR_ID_KEY + ":" + errorId.substr(1);
                        }
                        return content;
                    });

                ktestChunk[method].emplace_back(objects, status, errorDescriptors);
            }
        }
    }
    if (hasTimeout) {
        std::string message = StringUtils::stringFormat(
            "Some tests for function '%s' were skipped, as execution of function is "
            "out of timeout.",
            method.methodName);
        tests.commentBlocks.emplace_back(std::move(message));
    }
    if (hasError) {
        std::string message = StringUtils::stringFormat(
            "Some tests for function '%s' were skipped, as execution of function leads "
            "KLEE to the internal error. See console log for more details.",
            method.methodName);
        tests.commentBlocks.emplace_back(std::move(message));
    }

    if (!CollectionUtils::containsKey(ktestChunk, method) || ktestChunk.at(method).empty()) {
        tests.commentBlocks.emplace_back(StringUtils::stringFormat(
            "Tests for %s were not generated. Maybe the function is too complex.",
            method.methodName));
    }
}

std::pair<std::vector<std::string>, fs::path>
KleeRunner::createKleeParams(const tests::TestMethod &testMethod,
                             const tests::Tests &tests,
                             const std::string &methodNameOrEmptyForFolder) {
    fs::path kleeOut = Paths::kleeOutDirForEntrypoints(projectContext,
                                                       tests.sourceFilePath,
                                                       methodNameOrEmptyForFolder);
    fs::create_directories(kleeOut.parent_path());

    std::vector<std::string> argvData = { "klee",
                                          "--entry-point=" + KleeUtils::entryPointFunction(tests, testMethod.methodName, true),
                                          "--libc=klee",
                                          "--utbot",
                                          "--posix-runtime",
                                          "--type-system=CXX",
                                          "--fp-runtime",
                                          "--only-output-states-covering-new",
                                          "--allocate-determ",
                                          "--external-calls=all",
                                          "--timer-interval=1000ms",
                                          "--bcov-check-interval=8s",
                                          "-istats-write-interval=5s",
                                          "--disable-verify",
                                          "--check-div-zero=false",
                                          "--check-overshift=false",
                                          "--skip-not-lazy-and-symbolic-pointers",
                                          "--output-dir=" + kleeOut.string()};
    if (settingsContext.useDeterministicSearcher) {
        argvData.emplace_back("--search=dfs");
    }
    if (testMethod.is32bits) {
        // 32bit project
        argvData.emplace_back("--allocate-determ-size=" + std::to_string(1));
        argvData.emplace_back("--allocate-determ-start-address=" + std::to_string(0x10000));
    }
    return {argvData, kleeOut};
}

void KleeRunner::addTailKleeInitParams(std::vector<std::string> &argvData, const std::string &bitcodeFilePath)
{
    argvData.emplace_back(bitcodeFilePath);
    argvData.emplace_back("--sym-stdin");
    argvData.emplace_back(std::to_string(types::Type::symInputSize));
    argvData.emplace_back("--sym-files");
    argvData.emplace_back(std::to_string(types::Type::symFilesCount));
    argvData.emplace_back(std::to_string(types::Type::symInputSize));
}

void KleeRunner::processBatchWithoutInteractive(const std::vector<tests::TestMethod> &testMethods,
                                                tests::Tests &tests,
                                                std::vector<tests::MethodKtests> &ktests) {
    if (!tests.isFilePresentedInArtifact || testMethods.empty()) {
        return;
    }

    for (const auto &testMethod : testMethods) {
        if (testMethod.sourceFilePath != tests.sourceFilePath) {
            std::string message = StringUtils::stringFormat(
                "While generating tests for source file: %s tried to generate tests for method %s "
                "from another source file: %s. This can cause invalid generation.\n",
                tests.sourceFilePath, testMethod.methodName, testMethod.sourceFilePath);
            LOG_S(WARNING) << message;
        }

        auto [argvData, kleeOut] = createKleeParams(testMethod, tests, testMethod.methodName);
        addTailKleeInitParams(argvData, testMethod.bitcodeFilePath);
        {
            std::vector<char *> cargv, cenvp;
            std::vector<std::string> tmp;
            ExecUtils::toCArgumentsPtr(argvData, tmp, cargv, cenvp, false);
            LOG_S(DEBUG) << "Klee command :: " + StringUtils::joinWith(argvData, " ");
            MEASURE_FUNCTION_EXECUTION_TIME

            RunKleeTask task(cargv.size(), cargv.data(), settingsContext.timeoutPerFunction);
            ExecUtils::ExecutionResult result __attribute__((unused)) = task.run();
            ExecUtils::throwIfCancelled();

            MethodKtests ktestChunk;
            processMethod(ktestChunk, tests, kleeOut, testMethod);
            ktests.push_back(ktestChunk);
        }
    }
}

void KleeRunner::processBatchWithInteractive(const std::vector<tests::TestMethod> &testMethods,
                                             tests::Tests &tests,
                                             std::vector<tests::MethodKtests> &ktests) {
    if (!tests.isFilePresentedInArtifact || testMethods.empty()) {
        return;
    }

    for (const auto &method : testMethods) {
        if (method.sourceFilePath != tests.sourceFilePath) {
            std::string message = StringUtils::stringFormat(
                "While generating tests for source file: %s tried to generate tests for method %s "
                "from another source file: %s. This can cause invalid generation.\n",
                tests.sourceFilePath, method.methodName, method.sourceFilePath);
            LOG_S(WARNING) << message;
        }
    }

    auto [argvData, kleeOut] = createKleeParams(testMethods[0], tests, "");
    {
        // additional KLEE arguments
        argvData.emplace_back("--interactive");
        argvData.emplace_back(KleeUtils::processNumberOption());
        {
            // entrypoints
            fs::path entrypoints = kleeOut.parent_path() / "entrypoints.txt";
            std::ofstream of(entrypoints);
            for (const auto &method : testMethods) {
                of << KleeUtils::entryPointFunction(tests, method.methodName, true) << std::endl;
            }
            argvData.emplace_back("--entrypoints-file=" + entrypoints.string());
        }
        if (settingsContext.timeoutPerFunction.has_value()) {
            argvData.emplace_back(StringUtils::stringFormat(
                "--timeout-per-function=%d", settingsContext.timeoutPerFunction.value()));
        }
        addTailKleeInitParams(argvData, testMethods[0].bitcodeFilePath);
    }
    {
        std::vector<char *> cargv, cenvp;
        std::vector<std::string> tmp;
        ExecUtils::toCArgumentsPtr(argvData, tmp, cargv, cenvp, false);

        LOG_S(DEBUG) << "Klee command :: " + StringUtils::joinWith(argvData, " ");
        MEASURE_FUNCTION_EXECUTION_TIME

        RunKleeTask task(cargv.size(),
                         cargv.data(),
                         settingsContext.timeoutPerFunction.has_value()
                             ? settingsContext.timeoutPerFunction.value() * testMethods.size()
                             : settingsContext.timeoutPerFunction);
        ExecUtils::ExecutionResult result __attribute__((unused)) = task.run();

        ExecUtils::throwIfCancelled();

        for (const auto &method : testMethods) {
            std::string kleeMethodName =
                KleeUtils::entryPointFunction(tests, method.methodName, true);
            fs::path newKleeOut = kleeOut / kleeMethodName;
            MethodKtests ktestChunk;
            processMethod(ktestChunk, tests, newKleeOut, method);
            ktests.push_back(ktestChunk);
        }
    }
}
