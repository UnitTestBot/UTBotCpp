/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

/* eslint-disable no-async-promise-executor */
import * as grpc from 'grpc';
import * as vs from 'vscode';
import * as os from 'os';
import * as messages from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import { getProjectParams } from '../generators/gen';
import { utbotUI } from '../interface/utbotUI';
import { ExtensionLogger, LogLevel } from '../logger';
import { TestsGenServiceClient } from '../proto-ts/testgen_grpc_pb';
import {
    AssertionRequest,
    RegisterClientRequest,
    ConfigMode,
    CoverageAndResultsRequest,
    CoverageAndResultsResponse,
    DummyRequest,
    FunctionTypeResponse,
    HeartbeatResponse,
    LineRequest,
    LogChannelRequest,
    LogEntry,
    ProjectConfigRequest,
    ProjectConfigResponse,
    ProjectContext,
    ProjectTargetsResponse,
    FileTargetsResponse,
    SnippetRequest,
    StubsResponse,
    TestFilter,
    TestsResponse,
    FileTargetsRequest,
    ProjectTargetsRequest
} from '../proto-ts/testgen_pb';
import { SourceCode, SourceInfo, ValidationType } from '../proto-ts/util_pb';
import { RequestCoverageAndResultParams, RequestTestsParams } from '../requests/params';
import { Protos } from '../requests/protos';
import { Wrapper } from '../utils/utils';
import { ClientEventsEmitter } from './clientEventsEmitter';
import { ResponseHandler, SomeResponse } from '../responses/responseHandler';
const { logger, setupLogger } = ExtensionLogger;

type ResolveType<T> = (value: T | PromiseLike<T>) => void;
type RejectType = (reason?: any) => void;

enum ConnectionStatus {
    INIT, // Initial state of connection, when no request was sent
    BROKEN,
    ESTABLISHED
}

export class Client {
    private host!: string;
    private port!: number;
    private address!: string;

    private testsService!: TestsGenServiceClient;
    private grpcLogStream!: grpc.ClientReadableStream<LogEntry>;
    private grpcGTestStream!: grpc.ClientReadableStream<LogEntry>;

    private metadata: grpc.Metadata = new grpc.Metadata();

    private _ticker!: NodeJS.Timeout;
    get ticker(): NodeJS.Timeout {
        return this._ticker;
    }

    private connectionStatus: ConnectionStatus = ConnectionStatus.INIT;

    private _newClient = true;
    get newClient(): boolean {
        return this._newClient;
    }

    private logLevel: string;
    private workspaceId: string;
    readonly HEARTBEAT_INTERVAL = 500;
    readonly DEFAULT_LOG_LEVEL: LogLevel = 'INFO';
    readonly DAY_DURATION = 86400000; //milliseconds
    readonly DEFAULT_TIMEOUT = 300000;

    readonly events: ClientEventsEmitter;

    private noConnectionEstablishedBefore = true;

    constructor(private context: vs.ExtensionContext) {
        this.events = ClientEventsEmitter.getClientEventsEmitter();
        this.logLevel = this.DEFAULT_LOG_LEVEL;
        this.workspaceId = Prefs.obtainClientId(context);
    }

    public async setUpClient(): Promise<void> {
        this.noConnectionEstablishedBefore = true;
        // eslint-disable-next-line @typescript-eslint/camelcase
        if (process.env.https_proxy) {
            // eslint-disable-next-line @typescript-eslint/camelcase
            process.env.https_proxy = undefined;
        }

        this.host = Prefs.getHost();
        this.port = +Prefs.getPort();


        this.logLevel = this.DEFAULT_LOG_LEVEL;
        setupLogger(this.DEFAULT_LOG_LEVEL, utbotUI.channels().outputClientLogChannel);

        this.setEventsHandlers();
        this.initServices();
        await this.initHeartbeat();

    }

    public setEventsHandlers(): void {
        this.events.onDidHeartbeatFailureEventEmitter.on(errorMessage => {
            if (!this.isConnectionBroken()) {
                const logMesage = this.isConnectionInInitState()
                    ? `Connection with server can't be established`
                    : `Connection with server lost`;
                this.updateConnectionState(ConnectionStatus.BROKEN);
                messages.showErrorMessage(messages.serverIsDeadError);
                logger.warn(logMesage);
                logger.debug(errorMessage);
            }
        });

        this.events.onDidHeartbeatSuccessEventEmitter.on(async (response) => {
            if (this.noConnectionEstablishedBefore) {
                this.noConnectionEstablishedBefore = false;
                await this.events.onDidConnectFirstTimeEventEmitter.fire();
            }
            if (!this.isConnectionEstablished()) {
                messages.showInfoMessage(messages.successfullyConnected);
                logger.info('Successfully connected to server');
            }
            if (this.newClient || !response.getLinked()) {
                await Promise.all([
                    this.provideLogChannel(),
                    this.provideGTestChannel()
                ]);
                this._newClient = false;
            }
            this.updateConnectionState(ConnectionStatus.ESTABLISHED);
        });

        this.events.onDidHandshakeFailureEventEmitter.on(errorMessage => {
            logger.error(`Handshake error: ${errorMessage}`);
        });

        this.events.onDidHandshakeSuccessEventEmitter.on(response => {
            logger.info(`Handshake successfull`);
            logger.debug(`Handshake successfull response: ${response}`);
        });
    }

    public isConnectionBroken(): boolean {
        return this.connectionStatus === ConnectionStatus.BROKEN;
    }

    public isConnectionEstablished(): boolean {
        return this.connectionStatus === ConnectionStatus.ESTABLISHED;
    }

    public isConnectionInInitState(): boolean {
        return this.connectionStatus === ConnectionStatus.INIT;
    }

    public isConnectionNotEstablished(): boolean {
        return !this.isConnectionEstablished();
    }

    private async initHeartbeat(): Promise<void> {
        if (this.ticker !== null && this.ticker !== undefined) {
            clearInterval(this.ticker);
        }
        await this.heartbeat().catch(err =>
            logger.error(`Initial heartbeat error occurred ${err}`)
        );
        this._ticker = setInterval(() =>
            this.heartbeat().catch(err =>
                logger.error(`Error in heartbeat occurred ${err}`)),
            this.HEARTBEAT_INTERVAL);
    }

    private initServices(): void {
        this.address = this.host + ':' + this.port;
        logger.info(`Address is ${this.address}`);
        this.testsService = new TestsGenServiceClient(this.address,
            grpc.credentials.createInsecure());
    }

    private async setMetadata(): Promise<void> {
        return new Promise((resolve) => {
            let clientId = process.env['USER'];
            if (!clientId) {
                clientId = os.userInfo().username;
            }
            const registerClientRequest = new RegisterClientRequest();
            if (!clientId) {
                clientId = '';
            }
            clientId += '-' + this.workspaceId;
            registerClientRequest.setClientid(clientId);
            return new Promise<void>((resolve, reject) => {
                this.testsService.registerClient(registerClientRequest, (err) => {
                    if (err) {
                        reject(err.message);
                    } else {
                        resolve();
                    }
                });
            }).then(() => {
                this.metadata = new grpc.Metadata();
                this.metadata.add('clientId', clientId as string);
                resolve();
            }).then(undefined, err => {
                logger.error(err);
                resolve();
            });
        });
    }

    private async writeLog(responseAny: any): Promise<void> {
        const logEntry = responseAny as LogEntry;
        utbotUI.channels().outputServerLogChannel.append(logEntry.getMessage());
    }

    private async writeGTestLog(responseAny: any): Promise<void> {
        const gtestEntry = responseAny as LogEntry;
        utbotUI.channels().outputGTestChannel.show(true);
        utbotUI.channels().outputGTestChannel.appendLine(gtestEntry.getMessage());
    }

    private async provideLogChannel(): Promise<void> {
        const logChannelRequest = new LogChannelRequest();
        logChannelRequest.setLoglevel(this.logLevel);
        return new Promise<void>((resolve) => {
            const dummyRequest = new DummyRequest();
            this.testsService.closeLogChannel(dummyRequest, this.metadata, (err) => {
                if (err) {
                    logger.error(`Error while closing server log channel: ${err.message}`);
                }
            });
            resolve();
        }).then(() => new Promise<void>((resolve) => {
            this.grpcLogStream = this.testsService.openLogChannel(logChannelRequest, this.metadata);
            this.grpcLogStream.on('data', async responseAny => {
                await this.writeLog(responseAny);
            });
            resolve();
        }));
    }

    private async provideGTestChannel(): Promise<void> {
        const logChannelRequest = new LogChannelRequest();
        logChannelRequest.setLoglevel('MAX');
        return new Promise<void>((resolve) => {
            const dummyRequest = new DummyRequest();
            this.testsService.closeGTestChannel(dummyRequest, this.metadata, (err) => {
                if (err) {
                    logger.error(`Error while closing GTest channel: ${err.message}`);
                }
            });
            resolve();
        }).then(() => new Promise<void>((resolve) => {
            this.grpcGTestStream = this.testsService.openGTestChannel(logChannelRequest, this.metadata);
            this.grpcGTestStream.on('data', async responseAny => {
                await this.writeGTestLog(responseAny);
            });
            resolve();
        }));
    }

    async getLoggingLevel(): Promise<LogLevel | undefined> {
        const result = await vs.window.showQuickPick(['ERROR', 'INFO', 'DEBUG'], {
            placeHolder: 'Choose Logging Level for UTBot',
        });
        const resultAsLogLevel = result as LogLevel | undefined;
        return resultAsLogLevel;
    }

    async selectLoggingLevel(): Promise<void> {
        // eslint-disable-next-line no-async-promise-executor
        return new Promise(async (resolve) => {
            const newLogLevel = await this.getLoggingLevel();
            if (newLogLevel) {
                this.logLevel = newLogLevel;
                setupLogger(newLogLevel, utbotUI.channels().outputClientLogChannel);
            }
            await this.provideLogChannel();
            resolve();
        });
    }

    private updateConnectionState(connectionStatus: ConnectionStatus): void {
        this.connectionStatus = connectionStatus;
        utbotUI.indicators().connectionStatusBarItem.text =
            this.getTitleByConnectionStatus(connectionStatus);
    }

    private getTitleByConnectionStatus(connectionStatus: ConnectionStatus): string {
        switch (connectionStatus) {
            case ConnectionStatus.INIT:
                return utbotUI.titles.CONNECTION_SYNC;
            case ConnectionStatus.BROKEN:
                return utbotUI.titles.CONNECTION_DEAD;
            case ConnectionStatus.ESTABLISHED:
                return utbotUI.titles.CONNECTION_ALIVE;
            default:
                throw new Error("Unknown status");
        }
    }

    async heartbeat(): Promise<void> {
        const dummyRequest = new DummyRequest();
        return this.setMetadata().then(() => {
                return new Promise<void>((resolve, reject) => {
                    this.testsService.heartbeat(dummyRequest, this.metadata, async (err, response) => {
                        if (err) {
                            await this.events.onDidHeartbeatFailureEventEmitter.fire(err.message);
                            reject(err.message);
                        } else {
                            const serverResponse = response as HeartbeatResponse;
                            await this.events.onDidHeartbeatSuccessEventEmitter.fire(serverResponse);
                            resolve();
                        }
                    });
                });
        }).then(undefined, () => {
            if (!this.isConnectionBroken()) {
                this.updateConnectionState(ConnectionStatus.BROKEN);
            }
        });

    }

    async makeHandshakeRequest(): Promise<string> {
        const dummyRequest = new DummyRequest();
        let resolved = false;
        logger.info(`Sending handshake request`);
        setTimeout(() => {
            if (!resolved) {
                throw new Error(`Handshake with server failed. Check preferences and try again.`);
            }
        }, this.DEFAULT_TIMEOUT);
        return new Promise<string>((resolve, reject) => {
            this.testsService.handshake(dummyRequest, async (err, response) => {
                if (err) {
                    await this.events.onDidHandshakeFailureEventEmitter.fire(err.message);
                    reject(err);
                } else {
                    await this.events.onDidHandshakeSuccessEventEmitter.fire(response);
                    resolved = true;
                    resolve("");
                }
            });
        });
    }

    async checkProjectConfigurationRequest(
        projectName: string,
        projectPath: string,
        buildDirRelativePath: string,
        cmakeOptions: string,
        configMode: ConfigMode,
        progressKey: utbotUI.ProgressKey,
        token: vs.CancellationToken,
        responseHandler: ResponseHandler<ProjectConfigResponse>): Promise<ProjectConfigResponse> {
        return new Promise<ProjectConfigResponse>(async (resolve, reject) => {
            const projectContext = new ProjectContext();
            projectContext.setProjectname(projectName);
            projectContext.setProjectpath(projectPath);
            projectContext.setBuilddirrelativepath(buildDirRelativePath);
            const projectConfigRequest = new ProjectConfigRequest();
            projectConfigRequest.setProjectcontext(projectContext);
            projectConfigRequest.setConfigmode(configMode);
            projectConfigRequest.setCmakeoptions(cmakeOptions);
            try {
                const response = this.testsService.configureProject(projectConfigRequest, this.metadata);
                await this.handleServerResponse(response, progressKey, token, resolve, reject, responseHandler);
            } catch (err) {
                messages.showErrorMessage(err.message);
                utbotUI.channels().outputServerLogChannel.show(true);
                reject(err);
            }
        });
    }

    async requestSnippetTests(
        filepath: string,
        progressKey: utbotUI.ProgressKey,
        token: vs.CancellationToken,
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(`Sending snippet tests request, filepath: ${filepath}`);

        return new Promise<TestsResponse>(async (resolve, reject) => {
            const rpcRequest = new SnippetRequest();
            rpcRequest.setFilepath(filepath);

            const buildDir = Prefs.getBuildDirPath();
            const projectContext = new ProjectContext();
            projectContext.setProjectname(Prefs.getProjectName());
            projectContext.setProjectpath(buildDir[0]);
            projectContext.setTestdirpath(Prefs.getTestsDirPath());
            projectContext.setBuilddirrelativepath(buildDir[1]);
            rpcRequest.setProjectcontext(projectContext);
            rpcRequest.setSettingscontext(Prefs.getSettingsContext());

            try {
                const serverResponse = this.testsService.generateSnippetTests(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, progressKey, token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestFunctionTests(
        params: RequestTestsParams, 
        lineInfo: [string, number],
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(
            `Sending function tests request \n` +
            `lineInfo: [${lineInfo[0]} ${lineInfo[1]}]`
        );

        return new Promise<TestsResponse>(async (resolve, reject) => {
            const projectInfo = Protos.projectRequestByParams(params);
            const testLineInfo = Protos.sourceInfo(lineInfo);
            const rpcRequest = Protos.testsGenFunctionRequest(projectInfo, testLineInfo);
            try {
                const serverResponse = this.testsService.generateFunctionTests(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestFunctionReturnType(params: RequestTestsParams, lineInfo: [string, number]): Promise<FunctionTypeResponse> {
        logger.info(
            `Sending function return type request \n` +
            `lineInfo: [${lineInfo[0]} ${lineInfo[1]}]`
        );

        return new Promise(async (resolve, reject) => {
            const projectInfo = Protos.projectRequestByParams(params);
            const testLineInfo = Protos.sourceInfo(lineInfo);
            const rpcRequest = Protos.testsGenFunctionRequest(projectInfo, testLineInfo);
            const serverResponse = this.testsService.getFunctionReturnType(rpcRequest, this.metadata, (err, response) => {
                if (err) {
                    logger.error(`getFunctionReturnType error: ${err.message}`);
                    reject(err);
                } else {
                    resolve(response);
                }
            });
            return serverResponse;
        });
    }

    async requestClassTests(
        params: RequestTestsParams,
        lineInfo: [string, number],
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(
            `Sending class tests request \n` +
            `lineInfo: [${lineInfo[0]} ${lineInfo[1]}]`
        );

        return new Promise(async (resolve, reject) => {
            const projectInfo = Protos.projectRequestByParams(params);
            const testLineInfo = Protos.sourceInfo(lineInfo);
            const rpcRequest = Protos.testsGenClassRequest(projectInfo, testLineInfo);
            try {
                const serverResponse = this.testsService.generateClassTests(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestFileTests(
        params: RequestTestsParams, 
        filePath: string,
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(`Sending file tests request, filepath: ${filePath}`);

        return new Promise(async (resolve, reject) => {
            const projectInfo = Protos.projectRequestByParams(params);
            const rpcRequest = Protos.testsGenFileRequest(projectInfo, filePath);
            try {
                const serverResponse = this.testsService.generateFileTests(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestPredicateTests(
        params: RequestTestsParams,
        lineInfo: [string, number],
        predicateInfo: [ValidationType, string, string],
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(
            `Sending predicate tests request \n` +
            `lineInfo: [${lineInfo[0]} ${lineInfo[1]}], \n` +
            `predicateInfo: [${predicateInfo[1]} ${predicateInfo[2]}]`
        );

        return new Promise(async (resolve, reject) => {
            const projectInfo = Protos.projectRequestByParams(params);
            const testLineInfo = Protos.sourceInfo(lineInfo);
            const rpcPredicateRequest = Protos.testsGenPredicateRequest(projectInfo, testLineInfo, predicateInfo);
            try {
                const serverResponse = this.testsService.generatePredicateTests(rpcPredicateRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestsLineTests(
        params: RequestTestsParams,
        lineInfo: [string, number],
        needAssertion: boolean,
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(
            `Sending line tests request \n` +
            `lineInfo: [${lineInfo[0]} ${lineInfo[1]}]`
        );

        return await new Promise(async (resolve, reject) => {
            const projectRequest = Protos.projectRequestByParams(params);
            const testLineInfo = Protos.sourceInfo(lineInfo);
            const rpcRequest = new LineRequest();
            rpcRequest.setSourceinfo(testLineInfo);
            rpcRequest.setProjectrequest(projectRequest);

            try {
                let serverResponse: grpc.ClientReadableStream<TestsResponse>;
                if (needAssertion) {
                    const rpcRequestAssertion = new AssertionRequest();
                    rpcRequestAssertion.setLinerequest(rpcRequest);
                    serverResponse = this.testsService.generateAssertionFailTests(rpcRequestAssertion, this.metadata);
                } else {
                    serverResponse = this.testsService.generateLineTests(rpcRequest, this.metadata);
                }
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestFolderTests(
        params: RequestTestsParams,
        folderPath: string,
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(
            `Sending folder tests request \n` +
            `folderPath: ${folderPath}`
        );

        return new Promise(async (resolve, reject) => {
            const projectInfo = Protos.projectRequestByParams(params);
            const rpcRequest = Protos.testsGenFolderRequest(projectInfo, folderPath);
            try {
                const serverResponse = this.testsService.generateFolderTests(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestProjectTests(
        params: RequestTestsParams,
        responseHandler: ResponseHandler<TestsResponse>): Promise<TestsResponse> {
        logger.info(
            `Sending project tests request \n` +
            `projectName: ${params.projectName}\n` +
            `projectPath: ${params.projectPath}\n` +
            `buildDirRelativePath: ${params.buildDirRelativePath}\n` + 
            `targetPath: ${params.targetPath}`
        );

        return new Promise<TestsResponse>(async (resolve, reject) => {
            const rpcRequest = Protos.projectRequestByParams(params);
            try {
                const serverResponse = this.testsService.generateProjectTests(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestCoverageAndResults(
        params: RequestCoverageAndResultParams,
        responseHandler: ResponseHandler<CoverageAndResultsResponse>): Promise<CoverageAndResultsResponse> {
        const requestedTest = params.testInfo !== undefined ?
            `${params.testInfo.filePath}:${params.testInfo.testName}` :
            `ALL TESTS`;
        logger.info(
            `Sending 'Coverage and Results' request \n` +
            `test: ${requestedTest} \n`
        );

        return new Promise(async (resolve, reject) => {
            const rpcRequest = new CoverageAndResultsRequest();
            const projectContext = new ProjectContext();
            projectContext.setProjectname(Prefs.getProjectName());
            projectContext.setProjectpath(params.projectPath);
            projectContext.setTestdirpath(Prefs.getTestsDirPath());
            projectContext.setBuilddirrelativepath(params.buildDirRelativePath);
            rpcRequest.setProjectcontext(projectContext);
            rpcRequest.setSettingscontext(Prefs.getSettingsContext());
            rpcRequest.setCoverage(true);
            if (params.testInfo !== undefined) {
                const testFilter = new TestFilter();
                testFilter.setTestsuite(params.testInfo.testSuite);
                testFilter.setTestfilepath(params.testInfo.filePath);
                testFilter.setTestname(params.testInfo.testName);
                rpcRequest.setTestfilter(testFilter);
            }

            try {
                const serverResponse = this.testsService.createTestsCoverageAndResult(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.cancellationToken, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestFile(sourceInfo: SourceInfo): Promise<[SourceInfo, SourceCode]> {
        return new Promise<[SourceInfo, SourceCode]>((resolve, reject) => {
            this.testsService.getSourceCode(sourceInfo, this.metadata, (err, value) => {
                if (err) {
                    logger.error(`Request file error: ${err.message}`);
                    reject(err);
                } else {
                    resolve([sourceInfo, value]);
                }
            });
        });
    }

    requestFiles(sourceInfos: SourceInfo[]): Promise<[SourceInfo, SourceCode]>[] {
        return sourceInfos.map(async (sourceInfo) => {
            return this.requestFile(sourceInfo);
        });
    }

    async requestProjectStubs(
        params: RequestTestsParams,
        responseHandler: ResponseHandler<StubsResponse>): Promise<StubsResponse> {
        return new Promise(async (resolve, reject) => {
            const rpcRequest = Protos.projectRequestByParams(params);
            try {
                const serverResponse = this.testsService.generateProjectStubs(rpcRequest, this.metadata);
                await this.handleServerResponse(serverResponse, params.progressKey, params.token, resolve, reject, responseHandler);
            } catch (err) {
                reject(err);
            }
        });
    }

    async requestPrintModulesContent(): Promise<void> {
        await utbotUI.progresses().withProgress(async (progressKey, token) => {
            const params = getProjectParams(progressKey, token);
            const projectInfo = Protos.projectRequestByParams(params);
            return new Promise<void>((resolve, reject) => {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                this.testsService.printModulesContent(projectInfo.getProjectcontext()!, this.metadata, (err) => {
                    if (err) {
                        logger.error(`Print error: ${err.message}`);
                        reject(err);
                    } else {
                        resolve();
                    }
                });
            });
        });
    }

    async requestProjectTargets(
        progressKey: utbotUI.ProgressKey,
        token: vs.CancellationToken): Promise<ProjectTargetsResponse> {
        const params = getProjectParams(progressKey, token);
        const projectInfo = Protos.projectRequestByParams(params);
        return new Promise<ProjectTargetsResponse>((resolve, reject) => {
            const projectTargetsRequest = new ProjectTargetsRequest();
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            const projectContext = projectInfo.getProjectcontext()!;
            projectTargetsRequest.setProjectcontext(projectContext);
            this.testsService.getProjectTargets(projectTargetsRequest, this.metadata, (err, value) => {
                if (err) {
                    logger.error(`Request project targets error: ${err.message}`);
                    reject(err);
                } else {
                    resolve(value);
                }
            });
        });
    }

    async requestFileTargets(
        filePath: string,
        progressKey: utbotUI.ProgressKey,
        token: vs.CancellationToken): Promise<FileTargetsResponse> {
        const params = getProjectParams(progressKey, token);
        const projectInfo = Protos.projectRequestByParams(params);
        return new Promise<FileTargetsResponse>((resolve, reject) => {
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            const projectContext = projectInfo.getProjectcontext()!;
            const fileTargetsRequest = new FileTargetsRequest();
            fileTargetsRequest.setPath(filePath);
            fileTargetsRequest.setProjectcontext(projectContext);
            this.testsService.getFileTargets(fileTargetsRequest, this.metadata, (err, value) => {
                if (err) {
                    logger.error(`Request file targets error: ${err.message}`);
                    reject(err);
                } else {
                    resolve(value);
                }
            });
        });
    }

    private async handleServerResponse<T extends SomeResponse>(
        serverResponse: grpc.ClientReadableStream<SomeResponse>,
        progressKey: utbotUI.ProgressKey,
        token: vs.CancellationToken,
        resolve: ResolveType<T>,
        reject: RejectType,
        responseHandler: ResponseHandler<T>): Promise<void> {
        const started = new Wrapper<boolean>(false);
        setTimeout(() => {
            if (!started.value) {
                serverResponse.cancel();
                logger.info(`Request cancelled, timeout exceeded`);
                messages.showErrorMessage(messages.serverIsDeadError);
            }
        }, this.DEFAULT_TIMEOUT);
        serverResponse.on('data', async responseAny => {
            const response = responseAny as T;
            started.value = true;
            await this.handleResponse(response, progressKey, resolve, responseHandler);
        })
            .on('error', (err) => {
                started.value = true;
                utbotUI.channels().outputServerLogChannel.show(true);
                reject(err);
            });
        token.onCancellationRequested(() => {
            started.value = true;
            serverResponse.cancel();
            messages.showInfoMessage(messages.successfullyCanceledInfo);
        });
    }

    private async handleResponse<T extends SomeResponse>(
        response: T,
        progressKey: utbotUI.ProgressKey,
        resolve: ResolveType<T>,
        responseHandler: ResponseHandler<T>): Promise<void> {
        const progressInfo = response.getProgress();
        if (!progressInfo || progressInfo.getCompleted() === true) {
            if (progressInfo && progressInfo.getMessage()) {
                messages.showInfoMessage(progressInfo.getMessage());
            }
            resolve(response);
        } else {
            const message = progressInfo.getMessage();
            const increment = progressInfo.getPercent();
            if (increment >= 0.) {
                utbotUI.progresses().report(progressKey, message, increment);
                await responseHandler.handle(response);
            } else {
                utbotUI.progresses().report(progressKey, message);
            }
        }
    }
}

