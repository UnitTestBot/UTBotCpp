import { Client } from "../client/client";
import { Prefs } from "../config/prefs";
import { CoverageAndResultsResponse, ProjectConfigResponse, StubsResponse, TestsResponse } from "../proto-ts/testgen_pb";
import * as pathUtils from '../utils/pathUtils';
import * as vs from 'vscode';
import { ExtensionLogger } from "../logger";
import { TestsRunner } from "../runner/testsRunner";

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
        private readonly batched: boolean) {
    }

    public async handle(response: TestsResponse): Promise<void> {
        const testsSourceList = response.getTestsourcesList();
        testsSourceList.forEach(testsSourceInfo => {
            this.testsRunner.testResultsVizualizer.clearTestsByTestFileName(testsSourceInfo.getFilepath(), false);
        });
        if (Prefs.isRemoteScenario()) {
            const stubs = response.getStubs();
            if (stubs) {
                const stubsFiles = stubs.getStubsourcesList();
                await Promise.all(stubsFiles.map(async (stub) => {
                    const localPath = pathUtils.substituteLocalPath(stub.getFilepath());
                    const stubfile = vs.Uri.file(localPath);
                    logger.info(`Write stub file ${stub.getFilepath()} to ${localPath}`);
                    await vs.workspace.fs.writeFile(stubfile, Buffer.from(stub.getCode()));
                }));
            }
            const testsFiles = testsSourceList;
            await Promise.all((testsFiles).map(async (test) => {
                const localPath = pathUtils.substituteLocalPath(test.getFilepath());
                const testfile = vs.Uri.file(localPath);
                
                if (isTestFileSourceFile(testfile)) {
                    const testsNumberInErrorSuite = test.getErrormethodsnumber();
                    const testsNumberInRegressionSuite = test.getRegressionmethodsnumber();
                    logger.info(`Generated test file ${localPath} with ${testsNumberInRegressionSuite} tests in regression suite and ${testsNumberInErrorSuite} tests in error suite`);
                } else {
                    logger.info(`Generated test file ${localPath}`);
                }
                await vs.workspace.fs.writeFile(testfile, Buffer.from(test.getCode()));
                return testfile;
            }));
        }
        if (!this.batched) {
            const localPaths = testsSourceList.map(testsSourceInfo => pathUtils.substituteLocalPath(testsSourceInfo.getFilepath()));
            if (localPaths.length > 0) {
                const cppLocalPaths = localPaths.filter(fileName => fileName.endsWith('_test.cpp'));
                if (cppLocalPaths.length > 0) {
                    const fileToOpen = vs.Uri.file(cppLocalPaths[0]);
                    await vs.window.showTextDocument(fileToOpen, { preview: false });
                }
            }
        }
    }
}

function isTestFileSourceFile(testfile: any): boolean {
    return testfile.path.endsWith('_test.cpp');
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
