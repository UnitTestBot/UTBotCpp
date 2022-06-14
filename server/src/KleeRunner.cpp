#include "KleeRunner.h"

#include "Paths.h"
#include "exceptions/FileNotPresentedInArtifactException.h"
#include "exceptions/FileNotPresentedInCommandsException.h"
#include "tasks/RunKleeTask.h"
#include "utils/ExecUtils.h"
#include "utils/FileSystemUtils.h"
#include "utils/KleeUtils.h"
#include "utils/LogUtils.h"
#include "TimeExecStatistics.h"

#include "loguru.h"

#include <utility>

using namespace tests;

KleeRunner::KleeRunner(utbot::ProjectContext projectContext,
                       utbot::SettingsContext settingsContext,
                       fs::path serverBuildDir)
    : projectContext(std::move(projectContext)), settingsContext(std::move(settingsContext)),
      projectTmpPath(std::move(serverBuildDir)) {
}

void KleeRunner::runKlee(const std::vector<tests::TestMethod> &testMethods,
                         tests::TestsMap &testsMap,
                         const std::shared_ptr<KleeGenerator> &generator,
                         const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
                         const std::shared_ptr<LineInfo> &lineInfo,
                         TestsWriter *testsWriter,
                         bool isBatched,
                         bool interactiveMode) {
    LOG_SCOPE_FUNCTION(DEBUG);

    fs::path kleeOutDir = Paths::getKleeOutDir(projectTmpPath);
    if (fs::exists(kleeOutDir)) {
        FileSystemUtils::removeAll(kleeOutDir);
    }
    fs::create_directories(kleeOutDir);
    CollectionUtils::MapFileTo<std::vector<TestMethod>> fileToMethods;
    for (const auto &method : testMethods) {
        fileToMethods[method.sourceFilePath].push_back(method);
    }

    std::function<void(tests::Tests &tests)> writeFunctor = [&](tests::Tests &tests) {
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
            for (const auto &[methodName, bitcodeFile, sourceFilepath] : batch) {
                logStream << methodName << ", ";
            }
            LOG_S(MAX) << logStream.str();
        }
        if (interactiveMode) {
            if (!batch.empty()) {
                processBatchWithInteractive(batch, tests, ktests);
            }
        } else {
          for (auto const &testMethod : batch) {
              MethodKtests ktestChunk;
              processBatchWithoutInteractive(ktestChunk, testMethod, tests);
              ExecUtils::throwIfCancelled();
              ktests.push_back(ktestChunk);
          }
        }
        generator->parseKTestsToFinalCode(tests, methodNameToReturnTypeMap, ktests, lineInfo,
                                          settingsContext.verbose);
    };

    testsWriter->writeTestsWithProgress(testsMap, "Running klee", projectContext.testDirPath,
                                        std::move(writeFunctor));
}

fs::path KleeRunner::getKleeMethodOutFile(const TestMethod &method) {
    fs::path kleeOutDir = Paths::getKleeOutDir(projectTmpPath);
    fs::path relative =
        Paths::removeExtension(fs::relative(method.sourceFilePath, projectContext.projectPath));
    return kleeOutDir / relative / ("klee_out_" + method.methodName);
}

namespace {
    void clearUnusedData(const fs::path &kleeDir) {
        fs::remove(kleeDir / "assembly.ll");
        fs::remove(kleeDir / "run.istats");
    }

    void writeKleeStats(const fs::path &kleeOut) {
        ShellExecTask::ExecutionParameters kleeStatsParams("klee-stats",
                                                           { "--utbot-config", kleeOut.string() });
        auto [out, status, _] = ShellExecTask::runShellCommandTask(kleeStatsParams);
        if (status != 0) {
            LOG_S(ERROR) << "klee-stats call failed:";
            LOG_S(ERROR) << out;
        } else {
            LOG_S(DEBUG) << "klee-stats report:";
            LOG_S(DEBUG) << '\n' << out;
        }
    }
}

void KleeRunner::processBatchWithoutInteractive(MethodKtests &ktestChunk,
                                                const TestMethod &testMethod,
                                                Tests &tests) {
    if (!tests.isFilePresentedInArtifact) {
        return;
    }
    if (testMethod.sourceFilePath != tests.sourceFilePath) {
        std::string message = StringUtils::stringFormat(
                "While generating tests for source file: %s tried to generate tests for method %s "
                "from another source file: %s. This can cause invalid generation.\n", 
                tests.sourceFilePath, testMethod.methodName, testMethod.sourceFilePath);
        LOG_S(WARNING) << message;
    }

    std::string entryPoint = KleeUtils::entryPointFunction(tests, testMethod.methodName, true);
    std::string entryPointFlag = StringUtils::stringFormat("--entry-point=%s", entryPoint);
    auto kleeOut = getKleeMethodOutFile(testMethod);
    fs::create_directories(kleeOut.parent_path());
    std::string outputDir = "--output-dir=" + kleeOut.string();
    std::vector<std::string> argvData = { "klee",
                                          entryPointFlag,
                                          "--libc=klee",
                                          "--posix-runtime",
                                          "--fp-runtime",
                                          "--only-output-states-covering-new",
                                          "--allocate-determ",
                                          "--external-calls=all",
                                          "--timer-interval=1000ms",
                                          "--bcov-check-interval=6s",
                                          "-istats-write-interval=5s",
                                          "--disable-verify",
                                          "--check-div-zero=false",
                                          "--check-overshift=false",
                                          "--skip-not-lazy-and-symbolic-pointers",
                                          outputDir };
    if (settingsContext.useDeterministicSearcher) {
        argvData.emplace_back("--search=dfs");
    }
    argvData.push_back(testMethod.bitcodeFilePath);
    argvData.emplace_back("--sym-stdin");
    argvData.emplace_back(std::to_string(types::Type::symStdinSize));
    std::vector<char *> cargv, cenvp;
    std::vector<std::string> tmp;
    ExecUtils::toCArgumentsPtr(argvData, tmp, cargv, cenvp, false);
    LOG_S(DEBUG) << "Klee command :: " + StringUtils::joinWith(argvData, " ");
    MEASURE_FUNCTION_EXECUTION_TIME
    RunKleeTask task(cargv.size(), cargv.data(), settingsContext.timeoutPerFunction);
    ExecUtils::ExecutionResult result __attribute__((unused)) = task.run();
    ExecUtils::throwIfCancelled();

    if (fs::exists(kleeOut)) {
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
                    UTBotKTest::Status status = Paths::hasError(path) ? UTBotKTest::Status::FAILED
                                                                      : UTBotKTest::Status::SUCCESS;
                    std::vector<ConcretizedObject> kTestObjects(
                        ktestData->objects, ktestData->objects + ktestData->n_objects);

                    std::vector<UTBotKTestObject> objects = CollectionUtils::transform(
                        kTestObjects, [](const ConcretizedObject &kTestObject) {
                            return UTBotKTestObject{ kTestObject };
                        });

                    ktestChunk[testMethod].emplace_back(objects, status);
                }
            }
        }
        if (hasTimeout) {
            std::string message = StringUtils::stringFormat(
                "Some tests for function '%s' were skipped, as execution of function is "
                "out of timeout.",
                testMethod.methodName);
            tests.commentBlocks.emplace_back(std::move(message));
        }
        if (hasError) {
            std::string message = StringUtils::stringFormat(
                "Some tests for function '%s' were skipped, as execution of function leads "
                "KLEE to the internal error. See console log for more details.",
                testMethod.methodName);
            tests.commentBlocks.emplace_back(std::move(message));
        }
        writeKleeStats(kleeOut);
    }

    if (!CollectionUtils::containsKey(ktestChunk, testMethod) ||
        ktestChunk.at(testMethod).empty()) {
        tests.commentBlocks.emplace_back(StringUtils::stringFormat(
            "Tests for %s were not generated. Maybe the function is too complex.",
            testMethod.methodName));
    }
}

void KleeRunner::processBatchWithInteractive(const std::vector<tests::TestMethod> &testMethods,
                                             tests::Tests &tests,
                                             std::vector<tests::MethodKtests> &ktests) {
    if (!tests.isFilePresentedInArtifact) {
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

    TestMethod testMethod = testMethods[0];
    std::string entryPoint = KleeUtils::entryPointFunction(tests, testMethod.methodName, true);
    std::string entryPointFlag = StringUtils::stringFormat("--entry-point=%s", entryPoint);
    auto kleeOut = getKleeMethodOutFile(testMethod);
    fs::create_directories(kleeOut.parent_path());

    fs::path entrypoints = kleeOut.parent_path() / "entrypoints.txt";
    std::ofstream of(entrypoints);
    for (const auto &method : testMethods) {
        of << KleeUtils::entryPointFunction(tests, method.methodName, true) << std::endl;
    }
    of.close();
    std::string entrypointsArg = "--entrypoints-file=" + entrypoints.string();

    std::string outputDir = "--output-dir=" + kleeOut.string();
    std::vector<std::string> argvData = { "klee",
                                          entryPointFlag,
                                          "--libc=klee",
                                          "--posix-runtime",
                                          "--fp-runtime",
                                          "--only-output-states-covering-new",
                                          "--allocate-determ",
                                          "--external-calls=all",
                                          "--timer-interval=1000ms",
                                          "--bcov-check-interval=6s",
                                          "-istats-write-interval=5s",
                                          "--disable-verify",
                                          "--check-div-zero=false",
                                          "--check-overshift=false",
                                          "--skip-not-lazy-and-symbolic-pointers",
                                          "--interactive",
                                          KleeUtils::processNumberOption(),
                                          entrypointsArg,
                                          outputDir };
    if (settingsContext.timeoutPerFunction.has_value()) {
        argvData.push_back(StringUtils::stringFormat("--timeout-per-function=%d", settingsContext.timeoutPerFunction.value()));
    }
    if (settingsContext.useDeterministicSearcher) {
        argvData.emplace_back("--search=dfs");
    }
    argvData.push_back(testMethod.bitcodeFilePath);
    argvData.emplace_back("--sym-stdin");
    argvData.emplace_back(std::to_string(types::Type::symStdinSize));
    std::vector<char *> cargv, cenvp;
    std::vector<std::string> tmp;
    ExecUtils::toCArgumentsPtr(argvData, tmp, cargv, cenvp, false);

    LOG_S(DEBUG) << "Klee command :: " + StringUtils::joinWith(argvData, " ");
    MEASURE_FUNCTION_EXECUTION_TIME
    if (settingsContext.timeoutPerFunction.has_value()) {
        RunKleeTask task(cargv.size(), cargv.data(), settingsContext.timeoutPerFunction.value() * testMethods.size());
        ExecUtils::ExecutionResult result __attribute__((unused)) = task.run();
    } else {
        RunKleeTask task(cargv.size(), cargv.data(), settingsContext.timeoutPerFunction);
        ExecUtils::ExecutionResult result __attribute__((unused)) = task.run();
    }

    ExecUtils::throwIfCancelled();

    for (const auto &method : testMethods) {
        std::string kleeMethodName = KleeUtils::entryPointFunction(tests, method.methodName, true);
        fs::path newKleeOut = kleeOut / kleeMethodName;
        MethodKtests ktestChunk;
        if (fs::exists(newKleeOut)) {
            clearUnusedData(newKleeOut);
            bool hasTimeout = false;
            bool hasError = false;
            for (auto const &entry : fs::directory_iterator(newKleeOut)) {
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
                        UTBotKTest::Status status = Paths::hasError(path)
                                                    ? UTBotKTest::Status::FAILED
                                                    : UTBotKTest::Status::SUCCESS;
                        std::vector<ConcretizedObject> kTestObjects(
                            ktestData->objects, ktestData->objects + ktestData->n_objects);

                        std::vector<UTBotKTestObject> objects = CollectionUtils::transform(
                            kTestObjects, [](const ConcretizedObject &kTestObject) {
                              return UTBotKTestObject{ kTestObject };
                            });

                        ktestChunk[method].emplace_back(objects, status);
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
        }

        if (fs::exists(kleeOut)) {
            writeKleeStats(kleeOut);
        }

        if (!CollectionUtils::containsKey(ktestChunk, method) ||
            ktestChunk.at(method).empty()) {
            tests.commentBlocks.emplace_back(StringUtils::stringFormat(
                "Tests for %s were not generated. Maybe the function is too complex.",
                method.methodName));
        }
        ktests.push_back(ktestChunk);
    }
}
