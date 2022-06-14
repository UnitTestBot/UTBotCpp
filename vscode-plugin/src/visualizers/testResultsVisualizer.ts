import * as vs from "vscode";
import { GTestInfo, TestsCache } from "../cache/testsCache";
import { Prefs } from '../config/prefs';
import { DataLoader } from "../dataloader/dataLoader";
import { StateDecorationTypes } from "../interface/stateDecorationTypes";
import { ExtensionLogger } from "../logger";
import { TestResultObject, TestStatus } from "../proto-ts/testgen_pb";
import * as pathUtils from '../utils/pathUtils';
import { Visualizer } from "./visualizer";
const { logger } = ExtensionLogger;

export class TestWithStatus {
    constructor(public testInfo: GTestInfo, public testStatus: TestStatus){}
}

export class TestResultsVisualizer implements Visualizer, DataLoader<TestResultObject, TestWithStatus[]> {
    
    hidden: boolean;
    private testsWithStatuses: TestWithStatus[];
    private editors: Set<vs.TextEditor> = new Set();
    private decorations: StateDecorationTypes;

    constructor(context: vs.ExtensionContext) {
        this.hidden = false;
        this.testsWithStatuses = [];
        this.decorations = new StateDecorationTypes(context);
        this.subscribe();
    }

    private subscribe(): void {
        vs.window.onDidChangeActiveTextEditor(async (editor) => {
            if (editor) {
                await this.display(editor);
            }
        });
        vs.workspace.onDidDeleteFiles((event) => {
            event.files.forEach(fileUri => {
                this.clearTestsByTestFileName(fileUri.fsPath);
            });
        });
        vs.workspace.onDidChangeTextDocument((event) => {
            this.editors.forEach((editor) => {
                if (editor.document.uri === event.document.uri) {
                    this.hide(editor);
                    this.clearEditorTests(editor);
                }
            });
        });
    }

    public getLoadedData(): TestWithStatus[] {
        return this.testsWithStatuses;
    }

    public async loadData(data: Array<TestResultObject> | undefined = undefined): Promise<void> {
        if (data === undefined) {
            return;
        }
        const indexOfTest = (testResObj: TestResultObject): number => 
        this.testsWithStatuses.findIndex(test => 
            testResObj.getTestfilepath() === test.testInfo.filePath 
            && testResObj.getTestname() === test.testInfo.testName);
        for (const testResultObject of data) {
            let gtestInfo = TestsCache.getCache().get(testResultObject.getTestfilepath(), testResultObject.getTestname());
            if (gtestInfo === undefined) {
                gtestInfo = new GTestInfo(testResultObject.getTestfilepath(), "suite", testResultObject.getTestname(), -1);
            }
            const index = indexOfTest(testResultObject);
            if (index === -1) {
                const testInfo = new TestWithStatus(gtestInfo, testResultObject.getStatus());
                this.testsWithStatuses.push(testInfo);
            } else {
                this.testsWithStatuses[index].testStatus = testResultObject.getStatus();
                this.testsWithStatuses[index].testInfo = gtestInfo;
            }
        }
    }

    public clearData(): void {
        this.testsWithStatuses = [];
    }

    public clearEditorTests(editor: vs.TextEditor): void {
        this.clearTestsByTestFileName(editor.document.fileName, false);
    }

    public clearTestsByTestFileName(filename: string, canBeFolder: boolean = true): void {
        let serverFileName = filename;
        if (Prefs.isRemoteScenario()) {
            serverFileName = pathUtils.substituteRemotePath(serverFileName);
        }
        if (canBeFolder) {
            this.testsWithStatuses = this.testsWithStatuses.filter(test => !test.testInfo.filePath.startsWith(serverFileName));
        } else {
            this.testsWithStatuses = this.testsWithStatuses.filter(test => test.testInfo.filePath !== serverFileName);
        }
    } 

    private updateLines(editor: vs.TextEditor): void {
        const document = editor.document;
        for (let lineNumber = 0; lineNumber < document.lineCount; lineNumber++) {
            const line = document.lineAt(lineNumber);
            const startPos = line.firstNonWhitespaceCharacterIndex;
            if (line.text.slice(startPos).startsWith('TEST')) {
                // eslint-disable-next-line no-useless-escape
                const testArgs = line.text.match('[^()]+\((.*)\)');
                if (testArgs === null) {
                    continue;
                }
                // eslint-disable-next-line no-useless-escape
                const matches = testArgs[1].match(/([^(\s)\(\),]+)/g)?.map(obj => (obj === undefined) ? "" : obj);
                const testSuite: string = (matches === undefined) ? "" : matches[0];
                const testMethodName: string = (matches === undefined) ? "" : matches[1];
                let filename = document.fileName;
                if (Prefs.isRemoteScenario()) {
                    filename = pathUtils.substituteRemotePath(filename);
                }
                this.testsWithStatuses = this.testsWithStatuses.map(test => {
                    if (test.testInfo.filePath === filename && test.testInfo.testName === testMethodName) {
                        test.testInfo.lineNumber = lineNumber;
                    }
                    return test;
                });
                const testInfo = new GTestInfo(filename, testSuite, testMethodName, lineNumber);
                TestsCache.getCache().put(testInfo);
            }
        }
    }

    public async display(editor: vs.TextEditor): Promise<void> {
        if (!Prefs.showTestResults()) {
            return;
        }
        if (this.hidden) {
            return;
        }
        this.updateLines(editor);
        const failedRanges: vs.Range[] = [];
        const passedRanges: vs.Range[] = [];
        const deadRanges: vs.Range[] = [];
        let fileName = editor.document.fileName;
        if (Prefs.isRemoteScenario()) {
            fileName = pathUtils.substituteRemotePath(fileName);
        }
        this.testsWithStatuses
            .filter(test => test.testInfo.filePath === fileName)
            .forEach((test) => {
                if (test.testInfo.lineNumber === -1) {
                    const testfromCache = TestsCache.getCache().get(test.testInfo.filePath, test.testInfo.testName);
                    if (testfromCache !== undefined) {
                        test.testInfo.lineNumber = testfromCache.lineNumber;
                    } else {
                        logger.debug(`Test cache issue, test: ${test.testInfo.filePath}:${test.testInfo.testName}`);
                        // Test should be in cache
                    }
                }
                switch (test.testStatus) {
                    case TestStatus.TEST_FAILED:
                        failedRanges.push(new vs.Range(test.testInfo.lineNumber, 0, test.testInfo.lineNumber, 0));
                        break;
                    case TestStatus.TEST_PASSED:
                        passedRanges.push(new vs.Range(test.testInfo.lineNumber, 0, test.testInfo.lineNumber, 0));
                        break;
                    case TestStatus.TEST_DEATH:
                        deadRanges.push(new vs.Range(test.testInfo.lineNumber, 0, test.testInfo.lineNumber, 0));
                }
            });
        // TODO
        this.hide(editor);
        this.editors.add(editor);
        editor.setDecorations(this.decorations.failed, failedRanges);
        editor.setDecorations(this.decorations.passed, passedRanges);
        editor.setDecorations(this.decorations.dead, deadRanges);
    }

    hide(editor: vs.TextEditor): void {
        editor.setDecorations(this.decorations.failed, []);
        editor.setDecorations(this.decorations.passed, []);
        editor.setDecorations(this.decorations.dead, []);
        this.editors.delete(editor);
    }

    hideAll(): void {
        this.hidden = true;
        this.editors.forEach(editor => {
            this.hide(editor);
        });
    }
}