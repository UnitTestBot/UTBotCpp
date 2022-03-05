/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

/* eslint-disable no-case-declarations */
import * as vs from 'vscode';
import { Client } from '../client/client';
import * as messages from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import { ProjectConfig } from '../config/projectConfig';
import { utbotUI } from '../interface/utbotUI';
import { UTBotTargetsStorage } from '../explorer/utbotTargetsStorage';
import { ExtensionLogger } from '../logger';
import {ConfigMode, TestsResponse} from '../proto-ts/testgen_pb';
import { ValidationType } from "../proto-ts/util_pb";
import { RequestTestsParams } from '../requests/params';
import { TestsRunner } from '../runner/testsRunner';
import * as pathUtils from '../utils/pathUtils';
import * as vsUtils from '../utils/vscodeUtils';
import { getPredicate, getReturnValue } from '../validators/predicates';
import { UTBotProjectTarget } from '../explorer/UTBotProjectTarget';
import { ResponseHandler, SnippetResponseHandler, TestsResponseHandler } from '../responses/responseHandler';
const { logger } = ExtensionLogger;

export enum GenTestsMode {
    SingleSource,
    Project,
    Folder,
    File,
    Function,
    Class,
    Line,
    Assertion,
    Predicate
}

export function getProjectParams(
    progressKey: utbotUI.ProgressKey, 
    token: vs.CancellationToken,
    targetPath: string | undefined = undefined): RequestTestsParams {
    const projectDirName = Prefs.getProjectDirName();
    logger.debug(`Project dir name: ${projectDirName}`);
    const buildDirPath = Prefs.getBuildDirPath();
    logger.debug(`Build dir path: ${buildDirPath}`);
    const srcFilePaths = Prefs.getSourcePaths();
    logger.debug(`Source file paths: ${srcFilePaths.join(", ")}`);
    const targetPathToUse = targetPath !== undefined ? targetPath : UTBotTargetsStorage.instance.primaryTargetPath;

    const params = new RequestTestsParams(
        buildDirPath[0],
        buildDirPath[1],
        projectDirName,
        srcFilePaths,
        targetPathToUse ? targetPathToUse : "",
        progressKey,
        token
    );
    return params;
}

export async function run(
    mode: GenTestsMode,
    client: Client,
    testsRunner: TestsRunner,
    editor: vs.TextEditor | undefined = undefined,
    activeFilePath: string | undefined = undefined): Promise<void> {
    if (client.isConnectionNotEstablished() || client.newClient) {
        try {
            await client.heartbeat();
        } catch (error) {
            logger.error(`Heartbeat error: ${error}`);
        }
    }

    await new ProjectConfig(client).configure(ConfigMode.CHECK);

    let targetPath: string | undefined = undefined;
    do {
        targetPath = await runWithProgress(mode, client, testsRunner, editor, activeFilePath, targetPath);
    } while (targetPath !== undefined);
}

async function runWithProgress(
    mode: GenTestsMode,
    client: Client,
    testsRunner: TestsRunner,
    editor: vs.TextEditor | undefined,
    activeFilePath: string | undefined,
    targetPath: string | undefined = undefined
): Promise<string | undefined> {
    return utbotUI.progresses().withProgress<string | undefined>(async (progressKey, token) => {
        let params: RequestTestsParams;
        try {
            params = getProjectParams(progressKey, token, targetPath);
        } catch (err) {
            messages.showErrorMessage(err);
            return;
        }
        if (mode !== GenTestsMode.SingleSource && params.targetPath === "") {
            messages.showErrorMessage(messages.targetNotUsed);
            return;
        }
        try {
            let responseHandler: ResponseHandler<TestsResponse>;
            switch (mode) {
                case GenTestsMode.Project:
                    responseHandler = new TestsResponseHandler(client, testsRunner, true);
                    await client.requestProjectTests(params, responseHandler);
                    break;
                case GenTestsMode.File:
                    if (!activeFilePath) {
                        messages.showErrorMessage(messages.fileNotOpenedError);
                        return;
                    }
                    activeFilePath = pathUtils.substituteRemotePath(activeFilePath);
                    responseHandler = new TestsResponseHandler(client, testsRunner, false);
                    await client.requestFileTests(params, activeFilePath, responseHandler);
                    break;
                case GenTestsMode.Folder:
                    const fileUri = await getChosenFolder();
                    if (!fileUri) {
                        return;
                    }
                    const folderPath = pathUtils.substituteRemotePath(fileUri[0].fsPath);
                    responseHandler = new TestsResponseHandler(client, testsRunner, true);
                    await client.requestFolderTests(params, folderPath, responseHandler);
                    break;
                case GenTestsMode.Function:
                    const lineFunctionInfo = getLineInfo(editor);
                    responseHandler = new TestsResponseHandler(client, testsRunner, false);
                    await client.requestFunctionTests(params, lineFunctionInfo, responseHandler);
                    break;
                case GenTestsMode.Class:
                    const lineClassInfo = getLineInfo(editor);
                    responseHandler = new TestsResponseHandler(client, testsRunner, false);
                    await client.requestClassTests(params, lineClassInfo, responseHandler);
                    break;
                case GenTestsMode.Line:
                    const lineInfo = getLineInfo(editor);
                    responseHandler = new TestsResponseHandler(client, testsRunner, false);
                    await client.requestsLineTests(params, lineInfo, false, responseHandler);
                    break;
                case GenTestsMode.Assertion:
                    const lineInfoAssertion = getLineInfo(editor);
                    responseHandler = new TestsResponseHandler(client, testsRunner, false);
                    await client.requestsLineTests(params, lineInfoAssertion, true, responseHandler);
                    break;
                case GenTestsMode.Predicate:
                    const linePredicateInfo = getLineInfo(editor);
                    const returnType = await client.requestFunctionReturnType(params, linePredicateInfo);
                    const type = returnType.getValidationtype();
                    if (type === ValidationType.UNSUPPORTED) {
                        messages.showErrorMessage('Unsupported return type for \'Generate Tests With Prompted Result\' feature: ' +
                            'supported types are integers, booleans, characters, floats and strings');
                        break;
                    }
                    const predicate = await getPredicate(type);
                    if (!predicate) {
                        return;
                    }
                    const returnValue = await getReturnValue(type);
                    if (!returnValue) {
                        return;
                    }
                    const predicateInfo: [ValidationType, string, string] = [returnType.getValidationtype(), predicate, returnValue];
                    responseHandler = new TestsResponseHandler(client, testsRunner, false);
                    await client.requestPredicateTests(params, linePredicateInfo, predicateInfo, responseHandler);
                    break;
                default:
                    // noinspection ExceptionCaughtLocallyJS
                    throw new Error("Unexpected_Test_Generation_Mode");
            }
        } catch (err) {
            const { details, metadata } = err;
            if (details === 'File is not presented in compile/link commands for chosen target' ||
                details === 'File is presented in compile/link commands for chosen target, but not included in target\'s artifact') {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                const filePath = metadata.get("grpc-status-details-bin")!.toString(); //TODO proper unmarshalling of string
                const chooseTarget = 'Choose target';
                const skip = 'Skip';
                const fileTargets = await client.requestFileTargets(filePath, progressKey, token);
                const targetsList = fileTargets.getTargetsList();
                const message = `${details}: ${pathUtils.substituteLocalPath(filePath)}. See [documentation](https://github.com/UnitTestBot/UTBotCpp/wiki) for more details.`;
                return vs.window.showWarningMessage(message, ...[chooseTarget, skip]).then(async selection => {
                    if (selection === chooseTarget) {
                        const quickPickItems = targetsList
                            .map(projectTarget => UTBotProjectTarget.fromProjectTarget(projectTarget))
                            .sort(UTBotProjectTarget.compareFn)
                            .map(target => new utbotUI.TargetQuickPickItem(target));

                        const quickPickItem = await vs.window.showQuickPick(quickPickItems);
                        if (quickPickItem === undefined) {
                            messages.showWarningMessage('Alternative target was not chosen. Test generation canceled.');
                            return;
                        }
                        const newTargetPath = quickPickItem.target.path;
                        return newTargetPath;
                    }
                    if (selection === skip) {
                        messages.showInfoMessage('Test generation was skipped.');
                        return;
                    }
                });
            } else {
                messages.showErrorMessage(err);
                logger.error(err.message);
            }
            return;
        }
        testsRunner.coverageVizualizer.hideAll();
        testsRunner.testResultsVizualizer.hideAll();
    });
}

function getLineInfo(editor: vs.TextEditor | undefined): [string, number] {
    if (!editor) {
        throw new Error(messages.fileNotOpenedError);
    }
    const openedFile = pathUtils.substituteRemotePath(editor.document.uri.fsPath);
    const activeLine = editor.document.lineAt(editor.selection.active.line).lineNumber + 1;
    return [openedFile, activeLine];
}

export async function isolatedRun(client: Client, testsRunner: TestsRunner, activeFilePath: string | undefined): Promise<void> {
    await utbotUI.progresses().withProgress(async (progressKey, token) => {
        if (!activeFilePath) {
            messages.showErrorMessage(messages.fileNotOpenedError);
            return;
        }
        activeFilePath = pathUtils.substituteRemotePath(activeFilePath);
        const responseHandler = new SnippetResponseHandler(testsRunner);
        await client.requestSnippetTests(activeFilePath, progressKey, token, responseHandler).catch(
            async err => {
                messages.showErrorMessage(err +
                    ` Maybe you've run this command for file which actually is not isolated?`);
            }
        );
    });
}

async function getChosenFolder(): Promise<vs.Uri[] | undefined> {
    return vs.window.showOpenDialog({
        openLabel: 'Select a folder for which you need tests to be generated',
        canSelectFiles: false,
        canSelectFolders: true,
        canSelectMany: false,
        defaultUri: vsUtils.getProjectDirByOpenedFile()
    });
}