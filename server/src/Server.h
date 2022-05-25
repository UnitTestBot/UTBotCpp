/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SERVER_H
#define UNITTESTBOT_SERVER_H

#include "KleeGenerator.h"
#include "ThreadSafeContainers.h"
#include "TimeExecStatistics.h"
#include "exceptions/CancellationException.h"
#include "exceptions/CompilationDatabaseException.h"
#include "exceptions/ExecutionProcessException.h"
#include "exceptions/NoTestGeneratedException.h"
#include "printers/TestsPrinter.h"
#include "streams/stubs/StubsWriter.h"
#include "streams/tests/ServerTestsWriter.h"
#include "streams/tests/TestsWriter.h"
#include "testgens/AssertionTestGen.h"
#include "testgens/BaseTestGen.h"
#include "testgens/ClassTestGen.h"
#include "testgens/FileTestGen.h"
#include "testgens/FolderTestGen.h"
#include "testgens/FunctionTestGen.h"
#include "testgens/LineTestGen.h"
#include "testgens/PredicateTestGen.h"
#include "testgens/ProjectTestGen.h"
#include "testgens/SnippetTestGen.h"
#include "utils/LogUtils.h"
#include "utils/RequestLockMutex.h"
#include "utils/ServerUtils.h"
#include "utils/TimeUtils.h"

#include <grpcpp/grpcpp.h>
#include <parallel_hashmap/phmap.h>
#include <protobuf/testgen.grpc.pb.h>

#include "loguru.h"

#include "utils/path/FileSystemPath.h"
#include <future>
#include <string>
#include <variant>
#include <vector>

using json = nlohmann::json;

using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using namespace testsgen;

class Server {
public:
    Server();
    ~Server();

    explicit Server(bool testMode);

    void run(uint16_t customPort = 0);

    class TestsGenServiceImpl final : public TestsGenService::Service {
    public:
        TestsGenServiceImpl();

        explicit TestsGenServiceImpl(bool testMode);

        Status Handshake(ServerContext *context,
                         const DummyRequest *request,
                         DummyResponse *response) override;

        Status OpenLogChannel(ServerContext *context,
                              const LogChannelRequest *request,
                              ServerWriter<LogEntry> *writer) override;

        Status CloseLogChannel(ServerContext *context,
                               const DummyRequest *request,
                               DummyResponse *response) override;

        Status OpenGTestChannel(ServerContext *context,
                                const LogChannelRequest *request,
                                ServerWriter<LogEntry> *writer) override;

        Status CloseGTestChannel(ServerContext *context,
                               const DummyRequest *request,
                               DummyResponse *response) override;

        Status Heartbeat(ServerContext *context,
                         const DummyRequest *request,
                         HeartbeatResponse *response) override;

        Status PrintModulesContent(ServerContext *context,
                         const ProjectContext *request,
                         DummyResponse *response) override;

        Status RegisterClient(ServerContext *context,
                           const RegisterClientRequest *request,
                           DummyResponse *response) override;

        Status GetFunctionReturnType(ServerContext *context,
                                     const FunctionRequest *request,
                                     FunctionTypeResponse *response) override;

        template <typename TestGenT, typename RequestT>
        Status BaseTestGenerate(ServerContext *context,
                                RequestT const &request,
                                ServerWriter<TestsResponse> *writer) {
            static_assert(std::is_base_of<BaseTestGen, TestGenT>::value,
                          "Type parameter must derive from BaseTestGen");
            try {
                LOG_S(INFO) << typeid(RequestT).name() << " receive:\n" << request.DebugString();
                auto testsWriter = std::make_unique<ServerTestsWriter>(writer, GrpcUtils::synchronizeCode(request));

                ServerUtils::setThreadOptions(context, testMode);
                auto lock = acquireLock(testsWriter.get());

                MEASURE_FUNCTION_EXECUTION_TIME

                TestGenT testGen{ request, testsWriter.get(), testMode };
                if constexpr (std::is_base_of_v<ProjectTestGen, TestGenT>) {
                    fs::path targetPath = testGen.getRequest()->targetpath();
                    testGen.setTargetPath(targetPath);
                }
                TimeExecStatistics::clearStatistic();
                Status status = ProcessBaseTestRequest(testGen, testsWriter.get());
                TimeExecStatistics::printStatistic();
                return status;
            } catch (const CompilationDatabaseException &e) {
                return failedToLoadCDbStatus(e);
            }
        }

        Status GenerateSnippetTests(ServerContext *context,
                                    const SnippetRequest *request,
                                    ServerWriter<TestsResponse> *writer) override;

        Status GenerateProjectTests(ServerContext *context,
                                    const ProjectRequest *request,
                                    ServerWriter<TestsResponse> *writer) override;

        Status GenerateFileTests(ServerContext *context,
                                 const FileRequest *request,
                                 ServerWriter<TestsResponse> *writer) override;

        Status GenerateFunctionTests(ServerContext *context,
                                     const FunctionRequest *request,
                                     ServerWriter<TestsResponse> *writer) override;

        Status GenerateClassTests(ServerContext *context,
                                     const ClassRequest *request,
                                     ServerWriter<TestsResponse> *writer) override;

        Status GenerateFolderTests(ServerContext *context,
                                   const FolderRequest *request,
                                   ServerWriter<TestsResponse> *writer) override;

        Status GenerateLineTests(ServerContext *context,
                                 const LineRequest *request,
                                 ServerWriter<TestsResponse> *writer) override;

        Status GeneratePredicateTests(ServerContext *context,
                                      const PredicateRequest *request,
                                      ServerWriter<TestsResponse> *writer) override;

        Status GenerateAssertionFailTests(ServerContext *context,
                                          const AssertionRequest *request,
                                          ServerWriter<TestsResponse> *writer) override;

        Status CreateTestsCoverageAndResult(
            ServerContext *context,
            const CoverageAndResultsRequest *request,
            ServerWriter<::testsgen::CoverageAndResultsResponse> *writer) override;

        Status GenerateProjectStubs(ServerContext *context,
                                    const ProjectRequest *request,
                                    ServerWriter<StubsResponse> *writer) override;

        Status GetSourceCode(ServerContext *context,
                             const SourceInfo *request,
                             SourceCode *response) override;

        Status ConfigureProject(ServerContext *context,
                                const ProjectConfigRequest *request,
                                ServerWriter<ProjectConfigResponse> *response) override;

        Status GetProjectTargets(ServerContext *context,
                                 const ProjectTargetsRequest *request,
                                 ProjectTargetsResponse *response) override;

        Status GetFileTargets(ServerContext *context,
                              const FileTargetsRequest *request,
                              FileTargetsResponse *response) override;


        static Status ProcessBaseTestRequest(BaseTestGen &testGen, TestsWriter *testsWriter);

        static Status ProcessProjectStubsRequest(BaseTestGen *testGen, StubsWriter *stubsWriter);

        friend bool LogUtils::logChannelsWatcher(Server &server);
    private:
        std::mutex logChannelOperationsMutex;

        std::map <std::string, TimeUtils::systemClockTimePoint> linkedWithClient;
        std::map <std::string, std::atomic_bool> openedChannel;
        std::map <std::string, std::atomic_bool> openedGTestChannel;
        std::map <std::string, std::atomic_bool> holdLockFlag;
        concurrent_set<std::string> clients;

        template <class Key, class Value>
        using ConcurrentMap = phmap::node_hash_map<Key, Value>;

        ConcurrentMap<std::string, RequestLockMutex> locks;

        RequestLockMutex &getLock();

        std::unique_lock<RequestLockMutex> acquireLock(ProgressWriter *writer = nullptr);

        static std::shared_ptr<LineInfo> getLineInfo(LineTestGen &lineTestGen);

        static Status failedToLoadCDbStatus(const CompilationDatabaseException &e);

        Status provideLoggingCallbacks(const std::string &callbackPrefix,
                                       ServerWriter<LogEntry> *writer,
                                       const std::string &logLevel,
                                       loguru::log_handler_t handler,
                                       std::map<std::string, std::atomic_bool> &channelStorage,
                                       bool openFiles);

    protected:
        bool testMode = false;
    };

    TestsGenServiceImpl testsService;
    friend bool LogUtils::logChannelsWatcher(Server &server);
private:
    std::string host;
    uint16_t port;

    const static uint16_t DEFAULT_PORT = 2121;

    const static std::unordered_map<std::string, loguru::NamedVerbosity> verbosity;

    const static std::string logPrefix;
    const static std::string gtestLogPrefix;

    std::unique_ptr<grpc::Server> gRPCServer;

    std::atomic_bool logChannelsWatcherCancellationToken = false;
    std::future<bool> logChannelsWatcherTask;

    static uint16_t getPort();

    struct WriterData {
        ServerWriter<LogEntry> *writer;
        std::mutex writerMutex;
        const std::string client;
    };

    //recommended size from loguru
    const static size_t LOGURU_BUFFER_SIZE = 17;

    static void logToClient(void *channel, const loguru::Message &message);

    static void gtestLog(void *channel, const loguru::Message &message);
};


#endif //UNITTESTBOT_SERVER_H
