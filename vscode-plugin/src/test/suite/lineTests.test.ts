/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as assert from 'assert';
import * as path from 'path';
import * as vs from 'vscode';
import { Commands } from '../../config/commands';
import { executeCommand } from '../../utils/utils';
import {
    activate,
    checkCoverageJson,
    checkDirectoryWithTestsExists,
    checkTestFileNotEmpty,
    checkTestFilesGenerated,
    Compiler,
    openFile,
    restoreTestDirState,
    setTarget} from '../helper';

suite('"Generate Tests For Current Active Line" Test Suite', () => {
    const projectName = 'c-example-mini';
    const projectPath = path.resolve(__dirname,
        '../../../../', 'integration-tests', projectName);
    const openedFile = path.resolve(projectPath, 'lib', 'basic_functions.c');

    const targetName = 'liblib.a';

    const HeadOfMaxFunctionLineNumber = 6;
    const IfInMaxFunctionLineNumber = 7;

    async function checkAll(): Promise<void> {
        //TODO: get rid of the sleep once SAT-100 is done
        assert.ok(checkDirectoryWithTestsExists(projectPath));
        assert.ok(checkTestFilesGenerated(projectPath, ['basic_functions']));
        assert.ok(checkTestFileNotEmpty(projectPath));
        await executeCommand(Commands.RunAllTestsAndShowCoverage);
        assert.ok(checkCoverageJson(path.join(projectPath, 'build')));
    }

    async function prepare(compiler: Compiler, lineNumber: number | undefined): Promise<void> {
        await restoreTestDirState(projectPath, compiler);
        await activate(projectPath);
        await openFile(vs.Uri.file(openedFile), lineNumber);
        await setTarget(targetName);
    }

    test('[Happy Path]: generate test for max() fuction', async () => {
        const compiler = Compiler.Clang;
        await prepare(compiler, HeadOfMaxFunctionLineNumber);
        await executeCommand(Commands.GenerateProjectLineTests);
        await checkAll();
    });

    test('[Happy Path]: generate test for if statement inside max function', async () => {
        const compiler = Compiler.Clang;
        await prepare(compiler, IfInMaxFunctionLineNumber);
        await executeCommand(Commands.GenerateProjectLineTests);
        await checkAll();
    });

    test('[Happy Path]: generate test for if statement inside max function with gcc', async () => {
        const compiler = Compiler.Gcc;
        await prepare(compiler, IfInMaxFunctionLineNumber);
        await executeCommand(Commands.GenerateProjectLineTests);
        await checkAll();
    });
});
