#include "Server.h"

#include "BordersFinder.h"
#include "FeaturesFilter.h"
#include "GTestLogger.h"
#include "KleeRunner.h"
#include "ReturnTypesFetcher.h"
#include "Synchronizer.h"
#include "Version.h"
#include "building/Linker.h"
#include "building/UserProjectConfiguration.h"
#include "clang-utils/SourceToHeaderRewriter.h"
#include "coverage/CoverageAndResultsGenerator.h"
#include "exceptions/EnvironmentException.h"
#include "exceptions/FileNotPresentedInArtifactException.h"
#include "exceptions/FileNotPresentedInCommandsException.h"
#include "exceptions/FileSystemException.h"
#include "printers/CCJsonPrinter.h"
#include "printers/StubsPrinter.h"
#include "streams/FileTargetsWriter.h"
#include "streams/ProjectConfigWriter.h"
#include "streams/ProjectTargetsWriter.h"
#include "streams/TargetsWriter.h"
#include "streams/coverage/ServerCoverageAndResultsWriter.h"
#include "streams/stubs/ServerStubsWriter.h"
#include "streams/stubs/StubsWriter.h"
#include "stubs/StubGen.h"
#include "stubs/StubSourcesFinder.h"
#include "stubs/StubsCollector.h"
#include "utils/LogUtils.h"
#include "utils/ServerUtils.h"
#include "utils/TypeUtils.h"

#include <thread>
#include <fstream>

using TypeUtils::isSameType;

const std::string Server::logPrefix = "logTo";
const std::string Server::gtestLogPrefix = "gtestLogTo";

void Server::run(uint16_t customPort) {
    LOG_S(INFO) << "UnitTestBot Server, build " << UTBOT_BUILD_NUMBER;
    LOG_S(INFO) << "Logs directory: " << Paths::logPath;
    LOG_S(INFO) << "Latest log path: " << Paths::getUtbotLogAllFilePath();
    LOG_S(INFO) << "Tmp directory path: " << Paths::tmpPath;
    LOG_S(INFO) << "Executable path: " << fs::current_path();

    host = "0.0.0.0";
    if (customPort != 0) {
        port = customPort;
    } else {
        port = getPort();
    }
    std::string address = host + ":" + std::to_string(port);


    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&testsService);
    if (ServerUtils::checkPort(host, port)) {
        LOG_S(INFO) << "Address: " << address << std::endl;
        /* Launches the watcher in a separate thread that releases
         * unused grpc::ServerWriter<> resources.
         */
        logChannelsWatcherTask =
                std::async(std::launch::async, LogUtils::logChannelsWatcher, std::ref(*this));
        gRPCServer = builder.BuildAndStart();
        gRPCServer->Wait();
    } else {
        LOG_S(ERROR) << "Port unavailable: " << port << std::endl;
    }
}

uint16_t Server::getPort() {
    if (const char *envPort = std::getenv("UTBOT_SERVER_PORT")) {
        return std::stoi(envPort);
    }
    return DEFAULT_PORT;
}

Server::Server() {
}

Server::Server(bool testMode) : testsService(testMode) {
}

Server::~Server() {
    if (logChannelsWatcherTask.valid()) {
        logChannelsWatcherCancellationToken = true;
        std::future_status status;
        do {
            status = logChannelsWatcherTask.wait_for(TimeUtils::IDLE_TIMEOUT);
        } while (status != std::future_status::ready);
    }
}

Server::TestsGenServiceImpl::TestsGenServiceImpl() {
    ServerUtils::loadClientsData(clients);
}

Server::TestsGenServiceImpl::TestsGenServiceImpl(bool testMode) : TestsGenServiceImpl() {
    this->testMode = testMode;
}

Status Server::TestsGenServiceImpl::GenerateSnippetTests(ServerContext *context,
                                                         const SnippetRequest *request,
                                                         ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<SnippetTestGen, SnippetRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GenerateProjectTests(ServerContext *context,
                                                         const ProjectRequest *request,
                                                         ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<ProjectTestGen, ProjectRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GenerateFileTests(ServerContext *context,
                                                      const FileRequest *request,
                                                      ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<FileTestGen, FileRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GenerateFunctionTests(ServerContext *context,
                                                          const FunctionRequest *request,
                                                          ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<FunctionTestGen, FunctionRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GenerateClassTests(ServerContext *context,
                                                       const ClassRequest *request,
                                                       ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<ClassTestGen, ClassRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GenerateFolderTests(ServerContext *context,
                                                        const FolderRequest *request,
                                                        ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<FolderTestGen, FolderRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GenerateLineTests(ServerContext *context,
                                                      const LineRequest *request,
                                                      ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<LineTestGen, LineRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GenerateAssertionFailTests(
    ServerContext *context, const AssertionRequest *request, ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<AssertionTestGen, AssertionRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::GeneratePredicateTests(ServerContext *context,
                                                           const PredicateRequest *request,
                                                           ServerWriter<TestsResponse> *writer) {
    return BaseTestGenerate<PredicateTestGen, PredicateRequest>(context, *request, writer);
}

Status Server::TestsGenServiceImpl::Handshake(ServerContext *context,
                                              const DummyRequest *request,
                                              DummyResponse *response) {
    LOG_S(INFO) << "Handshake complete";
    return Status::OK;
}

Status Server::TestsGenServiceImpl::CreateTestsCoverageAndResult(
    ServerContext *context,
    const CoverageAndResultsRequest *request,
    ServerWriter<CoverageAndResultsResponse> *writer) {
    LOG_S(INFO) << "CreateTestsCoverageAndResult receive:\n" << request->DebugString();

    auto coverageAndResultsWriter = std::make_unique<ServerCoverageAndResultsWriter>(writer);

    ServerUtils::setThreadOptions(context, testMode);
    auto lock = acquireLock(coverageAndResultsWriter.get());
    Status status;
    {
        MEASURE_FUNCTION_EXECUTION_TIME
        CoverageAndResultsGenerator coverageGenerator(request, coverageAndResultsWriter.get());
        TimeExecStatistics::clearStatistic();
        auto settingsContext = utbot::SettingsContext(request->settingscontext());
        status = coverageGenerator.generate(request->coverage(), settingsContext);
        TimeExecStatistics::printStatistic();
    }
    return status;
}

// TODO: move to testgen base classes
Status Server::TestsGenServiceImpl::ProcessBaseTestRequest(BaseTestGen &testGen,
                                                           TestsWriter *testsWriter) {
    try {
        MEASURE_FUNCTION_EXECUTION_TIME
        types::TypesHandler::SizeContext sizeContext;

        static std::string logMessage = "Traversing sources AST tree and fetching declarations.";
        LOG_S(DEBUG) << logMessage;
        Fetcher fetcher(Fetcher::Options::Value::ALL,
                        testGen.compilationDatabase, testGen.tests, &testGen.types,
                        &sizeContext.pointerSize, &sizeContext.maximumAlignment,
                        testGen.compileCommandsJsonPath, false);
        fetcher.fetchWithProgress(testGen.progressWriter, logMessage);
        SourceToHeaderRewriter(testGen.projectContext, testGen.compilationDatabase,
                               fetcher.getStructsToDeclare(), testGen.serverBuildDir)
            .generateTestHeaders(testGen.tests, testGen.progressWriter);
        types::TypesHandler typesHandler{ testGen.types, sizeContext };
        testGen.progressWriter->writeProgress("Generating stub files", 0.0);
        StubGen stubGen(testGen);

        Synchronizer synchronizer(&testGen, &stubGen, &sizeContext);
        synchronizer.synchronize(typesHandler);

        std::shared_ptr<LineInfo> lineInfo = nullptr;
        auto lineTestGen = dynamic_cast<LineTestGen *>(&testGen);

        if (lineTestGen != nullptr) {
            if (isSameType<ClassTestGen>(testGen) && Paths::isHeaderFile(lineTestGen->filePath)) {
                BordersFinder classFinder(lineTestGen->filePath, lineTestGen->line,
                                          lineTestGen->compilationDatabase,
                                          lineTestGen->compileCommandsJsonPath);
                classFinder.findClass();
                lineInfo = std::make_shared<LineInfo>(classFinder.getLineInfo());
                lineInfo->filePath = lineTestGen->getSourcePath();
                CollectionUtils::erase_if(testGen.tests.at(lineInfo->filePath).methods,
                                          [&lineInfo](const tests::Tests::MethodDescription &methodDescription) {
                                              return methodDescription.isClassMethod() &&
                                                     methodDescription.classObj->type.typeName() != lineInfo->scopeName;
                                          });
            } else {
                lineInfo = getLineInfo(*lineTestGen);
                CollectionUtils::erase_if(testGen.tests.at(lineInfo->filePath).methods,
                                          [&lineInfo](const auto &methodDescription) {
                                              return methodDescription.name != lineInfo->methodName;
                                          });
            }
        }

        FeaturesFilter::filter(testGen.settingsContext, typesHandler, testGen.tests);
        StubsCollector(typesHandler).collect(testGen.tests);

        PathSubstitution pathSubstitution = {};
        if (lineTestGen != nullptr) {
            lineInfo->forMethod = isSameType<FunctionTestGen>(testGen);
            lineInfo->forClass = isSameType<ClassTestGen>(testGen);
            lineInfo->forAssert = isSameType<AssertionTestGen>(testGen);
            if (lineTestGen->needToAddPathFlag()) {
                LOG_S(DEBUG) << "Added test line flag for file " << lineInfo->filePath;
                fs::path flagFilePath =
                    printer::KleePrinter(&typesHandler, nullptr, Paths::getSourceLanguage(lineInfo->filePath))
                        .addTestLineFlag(lineInfo, lineInfo->forAssert, testGen.projectContext);
                pathSubstitution = { lineTestGen->filePath, flagFilePath };
            }
        }
        auto generator = std::make_shared<KleeGenerator>(
            testGen.projectContext, testGen.settingsContext,
            testGen.serverBuildDir, testGen.compilationDatabase, typesHandler,
            pathSubstitution, testGen.buildDatabase, testGen.progressWriter);

        ReturnTypesFetcher returnTypesFetcher{ &testGen };
        returnTypesFetcher.fetch(testGen.progressWriter, synchronizer.getAllFiles());
        LOG_S(DEBUG) << "Temporary build directory path: " << testGen.serverBuildDir;
        generator->buildKleeFiles(testGen.tests, lineInfo);
        generator->handleFailedFunctions(testGen.tests);
        testGen.progressWriter->writeProgress("Building files", 0.0);
        Linker linker{ testGen, stubGen, lineInfo, generator };
        linker.prepareArtifacts();
        auto testMethods = linker.getTestMethods();
        KleeRunner kleeRunner{ testGen.projectContext, testGen.settingsContext,
                               testGen.serverBuildDir };
        bool interactiveMode = (dynamic_cast<ProjectTestGen *>(&testGen) != nullptr);
        auto start_time = std::chrono::steady_clock::now();
        kleeRunner.runKlee(testMethods, testGen.tests, generator, testGen.methodNameToReturnTypeMap,
                           lineInfo, testsWriter, testGen.isBatched(), interactiveMode, testGen.settingsContext.errorMode);
        auto finish_time = std::chrono::steady_clock::now();
        LOG_S(INFO) << "KLEE time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish_time - start_time).count() << " ms\n";
    } catch (const ExecutionProcessException &e) {
        std::string command = e.what();
        return Status(StatusCode::FAILED_PRECONDITION,
                      "Executing command\n" + command.substr(0, 100) +
                          "...\nfailed. See more info in console logs.");
    } catch (const NoTestGeneratedException &e) {
        return Status(StatusCode::FAILED_PRECONDITION, e.what());
    } catch (const CancellationException &e) {
        return Status::CANCELLED;
    } catch (const NoSuchTypeException &e) {
        return Status(StatusCode::UNIMPLEMENTED, e.what());
    } catch (const EnvironmentException &e) {
        return Status(StatusCode::FAILED_PRECONDITION, e.what());
    } catch (const CompilationDatabaseException &e) {
        return Status(StatusCode::FAILED_PRECONDITION, e.what());
    } catch (const FileNotPresentedInCommandsException &e) {
        return Status(StatusCode::FAILED_PRECONDITION,
                      FileNotPresentedInCommandsException::MESSAGE, e.getFilePath());
    } catch (const FileNotPresentedInArtifactException &e) {
        return Status(StatusCode::FAILED_PRECONDITION,
                      FileNotPresentedInArtifactException::MESSAGE, e.getFilePath());
    } catch (const BaseException &e) {
        return Status(StatusCode::INTERNAL, e.what());
    }
    return Status::OK;
}

std::shared_ptr<LineInfo> Server::TestsGenServiceImpl::getLineInfo(LineTestGen &lineTestGen) {
    BordersFinder stmtFinder(lineTestGen.filePath, lineTestGen.line,
                             lineTestGen.compilationDatabase,
                             lineTestGen.compileCommandsJsonPath);
    stmtFinder.findFunction();
    if (!stmtFinder.getLineInfo().initialized) {
        throw NoTestGeneratedException(
            "Maybe you tried to generate tests placing cursor on invalid line.");
    }
    if (isSameType<AssertionTestGen>(lineTestGen) &&
        !StringUtils::contains(stmtFinder.getLineInfo().stmtString, "assert")) {
        throw NoTestGeneratedException("No assert found on this line.");
    }
    auto lineInfo = std::make_shared<LineInfo>(stmtFinder.getLineInfo());
    if (auto predicateInfo = dynamic_cast<PredicateTestGen *>(&lineTestGen)) {
        lineInfo->predicateInfo = LineInfo::PredicateInfo(
            { predicateInfo->type, predicateInfo->predicate, predicateInfo->returnValue });
    }
    auto &methods = lineTestGen.tests.at(lineInfo->filePath).methods;
    CollectionUtils::erase_if(methods, [&lineInfo](auto const &method) {
        return (lineInfo->forMethod && method.name != lineInfo->methodName) ||
               (lineInfo->forClass && method.isClassMethod() && method.classObj->type.typeName() != lineInfo->scopeName);
    });
    return lineInfo;
}

std::string extractMessage(const loguru::Message &message) {
    return std::string(message.preamble) + std::string(message.prefix) + message.message + "\n";
}

void Server::logToClient(void *channel, const loguru::Message &message) {
    auto data = reinterpret_cast<WriterData *>(channel);
    if (data == nullptr) {
        throw BaseException("Couldn't handle logging to client, data is null");
    }
    std::vector<char> thread_name(LOGURU_BUFFER_SIZE);
    loguru::get_thread_name(thread_name.data(), LOGURU_BUFFER_SIZE, false);

    if (std::string(thread_name.data()) == data->client &&
        std::string(message.filename) != std::string(GTestLogger::fileName())) {
        LogEntry logEntry;
        std::string extractedMessage = extractMessage(message);
        logEntry.set_message(extractedMessage);
        std::lock_guard<std::mutex> guard(data->writerMutex);
        data->writer->Write(logEntry);
    }
}

void Server::gtestLog(void *channel, const loguru::Message &message) {
    auto data = reinterpret_cast<WriterData *>(channel);
    if (data == nullptr) {
        throw BaseException("Can't interpret gtest log channel");
    }
    std::vector<char> thread_name(LOGURU_BUFFER_SIZE);
    loguru::get_thread_name(thread_name.data(), LOGURU_BUFFER_SIZE, false);

    if (std::string(thread_name.data()) == data->client &&
        std::string(message.filename) == std::string(GTestLogger::fileName())) {
        LogEntry logEntry;
        logEntry.set_message(message.message);
        std::lock_guard<std::mutex> guard(data->writerMutex);
        data->writer->Write(logEntry);
    }
}

loguru::Verbosity MaxNameToVerbosityCallback(const char* name) {
    if (strcmp(name, "TestLogLevel") == 0) {
        return loguru::Verbosity_INFO;
    } else if (strcmp(name, "ServerLogLevel") == 0) {
        return loguru::g_stderr_verbosity;
    }
    return loguru::Verbosity_INVALID;
}

Status Server::TestsGenServiceImpl::provideLoggingCallbacks(
    const std::string &callbackPrefix,
    ServerWriter<LogEntry> *writer,
    const std::string &logLevel,
    loguru::log_handler_t handler,
    std::map<std::string, std::atomic_bool> &channelStorage,
    bool openFiles) {
    const auto &client = RequestEnvironment::getClientId();
    auto oldValue = channelStorage[client].load(std::memory_order_relaxed);
    if (!oldValue && channelStorage[client].compare_exchange_weak(
                         oldValue, true, std::memory_order_release, std::memory_order_relaxed)) {
        WriterData data{ writer, std::mutex(), client };
        fs::path logFilePath = Paths::getLogDir();
        if (!fs::exists(logFilePath)) {
            fs::create_directories(logFilePath);
        }
        fs::path allLogPath = logFilePath / "everything.log";
        fs::path latestLogPath = logFilePath / "latest_readable.log";
        auto callbackName = callbackPrefix + client;
        loguru::set_name_to_verbosity_callback(&::MaxNameToVerbosityCallback);
        loguru::add_callback(callbackName.c_str(), handler, &data,
                             loguru::get_verbosity_from_name(logLevel.c_str()));
        if (openFiles) {
            loguru::add_file(allLogPath.c_str(), loguru::Append,
                             loguru::Verbosity_MAX);
            loguru::add_file(latestLogPath.c_str(), loguru::Truncate,
                             loguru::Verbosity_INFO);
        }
        holdLockFlag[callbackName] = true;
        /*
         * We use spinlocks here, because ServerWriter<LogEntry> *writer
         * is invalidated when Status::OK is sent, and therefore we can
         * not send logs to clients when they issue requests to UTBot. Possible
         * ways of redesigning logging system and removing the spinlock are:
         * 1. Using gRPC async API
         * 2. Issuing a request from UTBot to a specific client on every log entry.
         */
        while (holdLockFlag[callbackName].exchange(true, std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        loguru::remove_callback(callbackName.c_str());
        if (openFiles) {
            loguru::remove_callback(allLogPath.c_str());
            loguru::remove_callback(latestLogPath.c_str());
        }
        channelStorage[client] = false;
    }
    return Status::OK;
}

Status Server::TestsGenServiceImpl::OpenLogChannel(ServerContext *context,
                                                   const LogChannelRequest *request,
                                                   ServerWriter<LogEntry> *writer) {
    ServerUtils::setThreadOptions(context, testMode);
    return provideLoggingCallbacks(logPrefix, writer, request->loglevel(), logToClient, openedChannel,
                                   true);
}

Status Server::TestsGenServiceImpl::CloseLogChannel(ServerContext *context,
                                                    const DummyRequest *request,
                                                    DummyResponse *response) {
    ServerUtils::setThreadOptions(context, testMode);
    const std::string callbackName = logPrefix + RequestEnvironment::getClientId();
    holdLockFlag[callbackName].store(false, std::memory_order_release);
    return Status::OK;
}

Status Server::TestsGenServiceImpl::OpenGTestChannel(ServerContext *context,
                                                     const LogChannelRequest *request,
                                                     ServerWriter<LogEntry> *writer) {
    ServerUtils::setThreadOptions(context, testMode);
    return provideLoggingCallbacks(gtestLogPrefix, writer, request->loglevel(), gtestLog,
                                   openedGTestChannel, false);
}

Status Server::TestsGenServiceImpl::CloseGTestChannel(ServerContext *context,
                                                      const DummyRequest *request,
                                                      DummyResponse *response) {
    ServerUtils::setThreadOptions(context, testMode);
    const std::string callbackName = gtestLogPrefix + RequestEnvironment::getClientId();
    holdLockFlag[callbackName].store(false, std::memory_order_release);
    return Status::OK;
}


Status Server::TestsGenServiceImpl::Heartbeat(ServerContext *context,
                                              const DummyRequest *request,
                                              HeartbeatResponse *response) {
    ServerUtils::setThreadOptions(context, testMode);

    const std::string &client = RequestEnvironment::getClientId();
    const std::lock_guard<std::mutex> lock(logChannelOperationsMutex);
    bool linked = CollectionUtils::containsKey(linkedWithClient, client) &&
                  !TimeUtils::isOutdatedTimestamp(linkedWithClient[client]);
    response->set_linked(linked);
    linkedWithClient[client] = TimeUtils::now();
    return Status::OK;
}

Status Server::TestsGenServiceImpl::RegisterClient(ServerContext *context,
                                                   const RegisterClientRequest *request,
                                                   DummyResponse *response) {
    const std::string& name = request->clientid();
    ServerUtils::registerClient(clients, name);
    return Status::OK;
}

Status Server::TestsGenServiceImpl::GetFunctionReturnType(ServerContext *context,
                                                          const FunctionRequest *request,
                                                          FunctionTypeResponse *response) {
    LOG_S(INFO) << "GetFunctionReturnType receive:\n" << request->DebugString();

    ServerUtils::setThreadOptions(context, testMode);
    auto lock = acquireLock();

    MEASURE_FUNCTION_EXECUTION_TIME

    LineTestGen testGen(request->linerequest(), nullptr, testMode);
    auto lineInfo = getLineInfo(testGen);
    const auto &type = lineInfo->functionReturnType;
    testsgen::ValidationType typeResponse = testsgen::UNSUPPORTED;
    if (types::TypesHandler::isIntegerType(type)) {
        typeResponse = types::TypesHandler::getIntegerValidationType(type);
    } else if (types::TypesHandler::isBoolType(type)) {
        typeResponse = testsgen::BOOL;
    } else if (types::TypesHandler::isCharacterType(type)) {
        typeResponse = testsgen::CHAR;
    } else if (types::TypesHandler::isFloatingPointType(type)) {
        typeResponse = testsgen::FLOAT;
    } else if (types::TypesHandler::isCStringType(type) ||
               types::TypesHandler::isCppStringType(type)) {
        typeResponse = testsgen::STRING;
    }
    response->set_validationtype(typeResponse);
    return Status::OK;
}

Status Server::TestsGenServiceImpl::GenerateProjectStubs(ServerContext *context,
                                                         const ProjectRequest *request,
                                                         ServerWriter<StubsResponse> *writer) {
    try {
        LOG_S(INFO) << "GenerateProjectStubs receive:\n" << request->DebugString();

        auto stubsWriter =
            std::make_unique<ServerStubsWriter>(writer, GrpcUtils::synchronizeCode(*request));

        ServerUtils::setThreadOptions(context, testMode);
        auto lock = acquireLock(stubsWriter.get());

        MEASURE_FUNCTION_EXECUTION_TIME

        auto testGen = std::make_unique<ProjectTestGen>(*request, stubsWriter.get(), testMode);
        return ProcessProjectStubsRequest(testGen.get(), stubsWriter.get());
    } catch (const CompilationDatabaseException &e) {
        return failedToLoadCDbStatus(e);
    }
}

Status Server::TestsGenServiceImpl::ProcessProjectStubsRequest(BaseTestGen *testGen,
                                                               StubsWriter *stubsWriter) {
    types::TypesHandler::SizeContext sizeContext;
    types::TypesHandler typesHandler{ testGen->types, sizeContext };
    StubGen stubGen(*testGen);

    static std::string logMessage = "Traversing sources AST tree and fetching declarations.";
    LOG_S(DEBUG) << logMessage;
    Fetcher fetcher(Fetcher::Options::Value::TYPE | Fetcher::Options::Value::FUNCTION,
                    testGen->compilationDatabase, testGen->tests, &testGen->types,
                    &sizeContext.pointerSize, &sizeContext.maximumAlignment,
                    testGen->compileCommandsJsonPath, false);

    fetcher.fetchWithProgress(testGen->progressWriter, logMessage);
    Synchronizer synchronizer(testGen, &stubGen, &sizeContext);
    synchronizer.synchronize(typesHandler);
    stubsWriter->writeResponse(testGen->synchronizedStubs, testGen->projectContext.testDirPath);
    return Status::OK;
}

Status Server::TestsGenServiceImpl::failedToLoadCDbStatus(const CompilationDatabaseException &e) {
    return Status(StatusCode::INVALID_ARGUMENT,
                  "Failed to find compile_commands.json:\n" + std::string(e.what()));
}

Status Server::TestsGenServiceImpl::PrintModulesContent(ServerContext *context,
                                                        const ProjectContext *request,
                                                        DummyResponse *response) {
    LOG_S(INFO) << "PrintModulesContent receive:\n" << request->DebugString();

    ServerUtils::setThreadOptions(context, testMode);
    auto lock = acquireLock();

    MEASURE_FUNCTION_EXECUTION_TIME

    fs::path serverBuildDir = Paths::getTmpDir(request->projectname());
    utbot::ProjectContext projectContext{ *request };
    std::shared_ptr<BuildDatabase> buildDatabase = BuildDatabase::create(projectContext);
    StubSourcesFinder(buildDatabase).printAllModules();
    return Status::OK;
}

Status Server::TestsGenServiceImpl::GetSourceCode(ServerContext *context,
                                                  const SourceInfo *request,
                                                  SourceCode *response) {
    LOG_S(INFO) << "GetSourceCode receive:\n" << request->DebugString();

    ServerUtils::setThreadOptions(context, testMode);
    auto lock = acquireLock();

    MEASURE_FUNCTION_EXECUTION_TIME

    const std::string &filePath = request->filepath();
    std::ifstream stream{ filePath };
    if (!stream) {
        return Status(StatusCode::INVALID_ARGUMENT, "Failed to find file:\n" + filePath);
    }
    auto code = std::make_unique<std::string>(std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>());
    response->set_allocated_code(code.release());
    return Status::OK;
}

Status
Server::TestsGenServiceImpl::ConfigureProject(ServerContext *context,
                                              const ProjectConfigRequest *request,
                                              ServerWriter<ProjectConfigResponse> *response) {
    LOG_S(INFO) << "CheckProjectConfiguration receive:\n" << request->DebugString();
    ProjectConfigWriter writer{ response };

    ServerUtils::setThreadOptions(context, testMode);
    auto lock = acquireLock(&writer);

    MEASURE_FUNCTION_EXECUTION_TIME

    const auto &projectContext = request->projectcontext();
    fs::path buildDirPath =
        fs::path(projectContext.projectpath()) / projectContext.builddirrelativepath();
    switch (request->configmode()) {
    case ConfigMode::CHECK:
        return UserProjectConfiguration::CheckProjectConfiguration(buildDirPath, writer);
    case ConfigMode::CREATE_BUILD_DIR:
        return UserProjectConfiguration::RunBuildDirectoryCreation(buildDirPath, writer);
    case ConfigMode::GENERATE_JSON_FILES: {
        std::vector<std::string> cmakeOptions(request->cmakeoptions().begin(), request->cmakeoptions().end());
        return UserProjectConfiguration::RunProjectConfigurationCommands(
                buildDirPath, projectContext.projectname(), cmakeOptions, writer);
    }
    case ConfigMode::ALL: {
        std::vector<std::string> cmakeOptions(request->cmakeoptions().begin(), request->cmakeoptions().end());
        return UserProjectConfiguration::RunProjectReConfigurationCommands(
                buildDirPath, fs::path(projectContext.projectpath()),
                projectContext.projectname(), cmakeOptions, writer);
    }
    default:
        return {StatusCode::CANCELLED, "Invalid request type."};
    }
}

Status Server::TestsGenServiceImpl::GetProjectTargets(ServerContext *context,
                                                      const ProjectTargetsRequest *request,
                                                      ProjectTargetsResponse *response) {
    LOG_S(INFO) << "GetProjectTargets receive:\n" << request->DebugString();


    ServerUtils::setThreadOptions(context, testMode);
    auto lock = acquireLock();

    MEASURE_FUNCTION_EXECUTION_TIME

    try {
        utbot::ProjectContext projectContext{ request->projectcontext() };
        auto buildDatabase = BuildDatabase::create(projectContext);
        auto targets = buildDatabase->getAllTargets();
        ProjectTargetsWriter targetsWriter{ response };
        targetsWriter.writeResponse(projectContext, targets);
    } catch (CompilationDatabaseException const &e) {
        return failedToLoadCDbStatus(e);
    }
    return Status::OK;
}

Status Server::TestsGenServiceImpl::GetFileTargets(ServerContext *context,
                                                   const FileTargetsRequest *request,
                                                   FileTargetsResponse *response) {
    LOG_S(INFO) << "GetFileTargets receive:\n" << request->DebugString();

    ServerUtils::setThreadOptions(context, testMode);
    auto lock = acquireLock();

    MEASURE_FUNCTION_EXECUTION_TIME

    try {
        utbot::ProjectContext projectContext{ request->projectcontext() };
        auto buildDatabase = BuildDatabase::create(projectContext);
        fs::path path = request->path();
        auto targets = buildDatabase->getTargetsForSourceFile(path);
        FileTargetsWriter targetsWriter{ response };
        targetsWriter.writeResponse(targets, projectContext);
    } catch (CompilationDatabaseException const& e) {
        return failedToLoadCDbStatus(e);
    }
    return Status::OK;
}

RequestLockMutex &Server::TestsGenServiceImpl::getLock() {
    std::string const &client = RequestEnvironment::getClientId();
    auto [iterator, inserted] = locks.try_emplace(client);
    return iterator->second;
}

std::unique_lock<RequestLockMutex>
Server::TestsGenServiceImpl::acquireLock(ProgressWriter *writer) {
    auto &lock = getLock();
    if (lock.try_lock()) {
        return std::unique_lock{ lock, std::adopt_lock };
    }
    if (writer != nullptr) {
        writer->writeProgress("Waiting for previous task to be finished");
    }
    return std::unique_lock{ lock };
}
