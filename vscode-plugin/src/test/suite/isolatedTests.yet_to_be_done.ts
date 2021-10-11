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
        checkDirectoryWithTestsExists,
        checkTestFileNotEmpty,
        checkTestFilesGenerated,
        clearTestDir,
        openFile
} from '../helper';

suite('"Generate For Isolated File" Test Suite', () => {
        const projectName = 'c-example';
        const projectPath = path.resolve(__dirname,
                '../../../../', 'integration-tests', projectName);
        const srcFileRelativePath = 'snippet.c';
        const filePath = path.resolve(projectPath, srcFileRelativePath);

        // TODO: think about, we don't have a file as a result of test generation for a snippet
        test('Generate tests for a code snippet', async () => {
                await clearTestDir(projectPath);
                await activate(projectPath);
                await openFile(vs.Uri.file(filePath));
                await executeCommand(Commands.GenerateTestsForIsolatedFile);
                assert.ok(checkDirectoryWithTestsExists(projectPath));
                assert.ok(checkTestFilesGenerated(projectPath, ['snippet_test']));
                assert.ok(checkTestFileNotEmpty(projectPath));
        });
});