/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as assert from 'assert';
import * as path from 'path';
import * as vs from 'vscode';
import { Commands } from '../../config/commands';
import { Prefs } from '../../config/prefs';
import { executeCommand } from '../../utils/utils';
import {
    activate,
    checkCoverageJson,
    checkDirectoryWithTestsExists,
    checkTestFileNotEmpty,
    checkTestFilesGenerated,
    Compiler,
    openFile,
    PARAMETRIZED_TEST_MODE,
    restoreTestDirState,
    setTarget,
    VERBOSE_TEST_MODE
} from '../helper';

suite('"Generate Tests For Current File" Test Suite', () => {
    const projectName = 'c-example';
    const projectPath = path.resolve(__dirname,
        '../../../../', 'integration-tests', projectName);

    const fileName = 'simple_structs';
    const fileNameToTest = `${fileName}.c`;
    const openedFile = path.resolve(projectPath, 'lib', 'structures', 'structs', fileNameToTest);

    const targetName = 'liblib.a';

    const lineNumber = 2;

    async function checkAll(): Promise<void> {
        //TODO: get rid of the sleep once SAT-100 is done
        assert.ok(checkDirectoryWithTestsExists(projectPath));
        assert.ok(checkTestFilesGenerated(projectPath, [fileName]));
        assert.ok(checkTestFileNotEmpty(projectPath));
        await executeCommand(Commands.RunAllTestsAndShowCoverage);
        assert.ok(checkCoverageJson(path.join(projectPath, 'build')));
    }

    async function prepare(compiler: Compiler, verboseTestMode: boolean): Promise<void> {
        await restoreTestDirState(projectPath, compiler);
        await activate(projectPath);
        await Prefs.setVerboseTestMode(verboseTestMode);
        await openFile(vs.Uri.file(openedFile), lineNumber);
        await setTarget(targetName);
    }

    test(`[Happy Path]: generate test for ${fileNameToTest} file with parametrized mode`, async () => {
        const compiler = Compiler.Clang;
        await prepare(compiler, PARAMETRIZED_TEST_MODE);
        await executeCommand(Commands.GenerateTestsForFile);
        await checkAll();
    });

    test(`[Happy Path]: generate test for ${fileNameToTest} file with verbose mode`, async () => {
        const compiler = Compiler.Clang;
        await prepare(compiler, VERBOSE_TEST_MODE);
        await executeCommand(Commands.GenerateTestsForFile);
        await checkAll();
    });
});
