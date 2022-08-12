import { Client } from "../client/client";
import { Prefs } from "../config/prefs";
import { CoverageAndResultsResponse, ProjectConfigResponse, StubsResponse, TestsResponse } from "../proto-ts/testgen_pb";
import * as pathUtils from '../utils/pathUtils';
import * as vs from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { ExtensionLogger } from "../logger";
import { TestsRunner } from "../runner/testsRunner";
import {Uri} from "vscode";
import * as messages from "../config/notificationMessages";

const { logger } = ExtensionLogger;

export type SomeResponse = CoverageAndResultsResponse | TestsResponse | StubsResponse | ProjectConfigResponse;

export interface ResponseHandler<T extends SomeResponse> {
    handle(response: T): Promise<void>;
}

export class DummyResponseHandler<T extends SomeResponse> implements ResponseHandler<T> {
    public async handle(_response: T): Promise<void> {
        return;
    }
}

export class TestsResponseHandler implements ResponseHandler<TestsResponse> {
    constructor(
        private readonly client: Client,
        private readonly testsRunner: TestsRunner,
        private readonly generateForMultipleSources: boolean) {
    }

    public async handle(response: TestsResponse): Promise<void> {
        const testsSourceList = response.getTestsourcesList();

        // Delete/backup old info
        for (const test of testsSourceList) {
            const localPath = pathUtils.substituteLocalPath(test.getFilepath());
            if (isSarifReportFile(localPath)) {
                if (Prefs.isRemoteScenario()) {
                    // do not back up the SARIF for local scenario - server did it
                    await backupPreviousClientSarifReport(localPath);
                }
            }
            else {
                this.testsRunner.testResultsVizualizer.removeFileFromData(localPath, false);
            }
        }

        // Transfer files' code if need
        if (Prefs.isRemoteScenario()) {
            //  do not write files for local scenario - server did it
            const stubs = response.getStubs();
            if (stubs) {
                const stubsFiles = stubs.getStubsourcesList();
                for (const stub of stubsFiles) {
                    const localPath = pathUtils.substituteLocalPath(stub.getFilepath());
                    logger.info(`Write stub file ${stub.getFilepath()} to ${localPath}`);
                    await vs.workspace.fs.writeFile(vs.Uri.file(localPath), Buffer.from(stub.getCode()));
                }
            }
            for (const test of testsSourceList) {
                const localPath = pathUtils.substituteLocalPath(test.getFilepath());
                await vs.workspace.fs.writeFile(vs.Uri.file(localPath), Buffer.from(test.getCode()));
            }
        }

        // Show and log the results in UI
        {
            let firstTest = true;
            const SarifReportFiles: Uri[] = [];
            for (const test of testsSourceList) {
                const localPath = pathUtils.substituteLocalPath(test.getFilepath());

                if (isSarifReportFile(localPath)) {
                    logger.info(`Generated SARIF file ${localPath}`);
                    SarifReportFiles.push(vs.Uri.file(localPath));
                } else if (isTestFileSourceFile(localPath)) {
                    const testsNumberInErrorSuite = test.getErrormethodsnumber();
                    const testsNumberInRegressionSuite = test.getRegressionmethodsnumber();
                    logger.info(`Generated test file ${localPath} with ${testsNumberInRegressionSuite} tests in regression suite and ${testsNumberInErrorSuite} tests in error suite`);
                    if (!this.generateForMultipleSources && firstTest) {
                        // show generated test file for line, class, function, single source file
                        firstTest = false;
                        await vs.window.showTextDocument(vs.Uri.file(localPath), {preview: false});
                    }
                } else {
                    logger.info(`Generated test file ${localPath}`);
                }
            }
            if (SarifReportFiles.length > 0) {
                const sarifExt = vs.extensions.getExtension(messages.defaultSARIFViewer);
                // eslint-disable-next-line eqeqeq
                if (sarifExt == null) {
                    messages.showWarningMessage(messages.intstallSARIFViewer);
                } else {
                    if (!sarifExt.isActive) {
                        await sarifExt.activate();
                    }
                    // SARIF plugin doesn't monitor the file change event
                    // To refresh the report we close and open report
                    await sarifExt.exports.closeLogs(SarifReportFiles)
                        .then(sarifExt.exports.openLogs(SarifReportFiles));
                }
            }
        }
    }
}

function isSarifReportFile(testfile: string): boolean {
    return testfile.endsWith("project_code_analysis.sarif");
}

async function backupPreviousClientSarifReport(localPath: string): Promise<void> {
    if (fs.existsSync(localPath)) {
        const ctime = fs.lstatSync(localPath).ctime;

        // eslint-disable-next-line no-inner-declarations
        function pad2(num: number): string {
            return ("0" + num).slice(-2);
        }

        const newName = "project_code_analysis-"
            + ctime.getFullYear()
            + pad2(ctime.getMonth() + 1)
            + pad2(ctime.getDate())
            + pad2(ctime.getHours())
            + pad2(ctime.getMinutes())
            + pad2(ctime.getSeconds())
            + ".sarif";
        await vs.workspace.fs.rename(vs.Uri.file(localPath), Uri.file(path.join(path.dirname(localPath), newName)));
    }
}

function isTestFileSourceFile(localpath: string): boolean {
    return localpath.endsWith('_test.cpp');
}

export class SnippetResponseHandler implements ResponseHandler<TestsResponse> {
    constructor(
        private readonly testsRunner: TestsRunner) {
    }

    public async handle(response: TestsResponse): Promise<void> {
        this.testsRunner.hideTestResultsAndCoverage();
        const testsSourcesList = response.getTestsourcesList();
        if (testsSourcesList.length === 0) {
            return;
        }
        const testsSource = testsSourcesList[0];
        const code = testsSource.getCode();
        await vs.workspace.openTextDocument({ content: code, language: 'cpp' })
            .then(async doc => {
                await vs.window.showTextDocument(doc, { preview: false });
            });
    }
}
