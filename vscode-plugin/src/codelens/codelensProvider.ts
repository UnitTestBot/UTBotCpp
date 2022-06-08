import * as vs from 'vscode';
import { GTestInfo, TestsCache } from '../cache/testsCache';
import { Commands } from '../config/commands';
import * as messages from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import * as pathUtils from '../utils/pathUtils';

export class TestLensProvider implements vs.CodeLensProvider {

    private readonly RUN_TEST_LENS_TITLE = '$(run) Run Test With Coverage';
    private readonly RUN_TESTS_LENS_TITLE = '$(run-all) Run Tests With Coverage';

    private codeLenses: vs.CodeLens[] = [];

    private createCodeLens(lineNumber: number, filepath: string, testSuite: string, testMethodName: string, title: string): vs.CodeLens {
        const codeLens = new vs.CodeLens(new vs.Range(lineNumber, 0, lineNumber + 1, 0));
        const testInfo = new GTestInfo(filepath, testSuite, testMethodName, lineNumber);
        TestsCache.getCache().put(testInfo);
        codeLens.command = {
            title: title,
            command: Commands.RunSpecificTestAndShowCoverage,
            arguments: [testInfo]
        };
        return codeLens;
    }

    public provideCodeLenses(document: vs.TextDocument, _token: vs.CancellationToken): vs.CodeLens[] | Thenable<vs.CodeLens[]> {
        this.codeLenses = [];
        let testsDirPath: string;
        try {
            testsDirPath = Prefs.getLocalTestsDirPath();
        } catch (err) {
            messages.showErrorMessage(err);
            return [];
        }
        // provide lenses only for test files
        if (!document.uri.fsPath.startsWith(testsDirPath)) {
            return [];
        }
        return new Promise<vs.CodeLens[]>((resolve) => {
            let filename = document.fileName;
            if (Prefs.isRemoteScenario()) {
                filename = pathUtils.substituteRemotePath(filename);
            }
            for (let lineNumber = 0; lineNumber < document.lineCount; lineNumber++) {
                const line = document.lineAt(lineNumber);
                const startPos = line.firstNonWhitespaceCharacterIndex;
                if (line.text.startsWith('TEST', startPos)) {
                    // eslint-disable-next-line no-useless-escape
                    const testArgs = line.text.match('[^()]+\((.*)\)');
                    if (testArgs === null) {
                        continue;
                    }
                    // eslint-disable-next-line no-useless-escape
                    const matches = testArgs[1].match(/([^(\s)\(\),]+)/g)?.map(obj => (obj === undefined) ? "" : obj);
                    const testSuite: string = (matches === undefined) ? "" : matches[0];
                    const testMethodName: string = (matches === undefined) ? "" : matches[1];
                    const codeLensRunTest = this.createCodeLens(lineNumber, filename, testSuite, testMethodName, this.RUN_TEST_LENS_TITLE);
                    this.codeLenses.push(codeLensRunTest);
                }
                if (line.text.startsWith('namespace UTBot', startPos)) {
                    const codeLensRunTests = this.createCodeLens(lineNumber, filename, "", "", this.RUN_TESTS_LENS_TITLE);
                    this.codeLenses.push(codeLensRunTests);
                }
            }
            resolve(this.codeLenses);
        });
    }

    public resolveCodeLens(codeLens: vs.CodeLens, _token: vs.CancellationToken): vs.CodeLens {
        return codeLens;
    }
}
