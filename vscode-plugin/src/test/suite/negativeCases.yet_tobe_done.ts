/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as assert from 'assert';
import * as path from 'path';
import * as vs from 'vscode';
import { Commands } from '../../config/commands';
import { executeCommand } from '../../utils/utils';
import {
        checkCoverageJson,
        checkDirectoryWithTestsExists,
        checkTestFileNotEmpty,
        checkTestFilesGenerated,
        Compiler,
        restoreTestDirState
} from '../helper';

suite('"Generate Tests For Project" Test Suite', () => {
        const projectName = 'c-example-mini';
        const projectPath = path.resolve(__dirname,
                '../../../../', 'integration-tests', projectName);

        // eslint-disable-next-line @typescript-eslint/no-unused-vars
        async function checkAll(): Promise<void> {
            assert.ok(checkDirectoryWithTestsExists(projectPath));
            assert.ok(checkTestFilesGenerated(projectPath, ['assertion_failures', 'basic_functions', 'dependent_functions', 'pointer_parameters']));
            assert.ok(checkTestFileNotEmpty(projectPath));
            await executeCommand(Commands.RunAllTestsAndShowCoverage);
            assert.ok(checkCoverageJson(path.join(projectPath, 'build')));
        }

        //TODO: there is an issue with await/async calls inside the plugin code, needs to be checked separately
        test('[Negative Case]: Wrong settings for the server socket', async () => {
               await restoreTestDirState(projectPath, Compiler.Clang);
               await vs.workspace.getConfiguration().update('unittestbot.deployment.utbotHost', '228228');
               await executeCommand(Commands.GenerateTestsForProject);
               await vs.workspace.getConfiguration().update('unittestbot.deployment.utbotHost', '127.0.0.1');
               try{
                   await executeCommand(Commands.GenerateTestsForProject);
               } catch(ignored){}
               assert.fail;
        });
});
