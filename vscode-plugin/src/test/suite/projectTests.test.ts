/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as assert from 'assert';
import * as path from 'path';
import * as vs from 'vscode';
import { Commands } from '../../config/commands';
import { Prefs } from '../../config/prefs';
import { UTBotTargetsStorage } from '../../explorer/utbotTargetsStorage';
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
        setTarget,
        VERBOSE_TEST_MODE
} from '../helper';

suite('"Generate Tests For Project" Test Suite', () => {
        const projectName = 'c-example';
        const projectPath = path.resolve(__dirname,
                '../../../../', 'integration-tests', projectName);
        const openedFile = path.resolve(projectPath, 'lib', 'basic_functions.c');

        async function checkAll(): Promise<void> {
                assert.ok(checkDirectoryWithTestsExists(projectPath));
                    assert.ok(checkTestFilesGenerated(projectPath, [
                        'assertion_failures', 'basic_functions',
                        'dependent_functions', 'inner_basic_functions',
                        'complex_structs', 'main', 'pointer_return',
                        'struct_arrays', 'pointer_parameters',
                        'simple_structs', 'types', 'typedefs_1', 'simple_unions',
                        'typedefs_2', 'enums', 'packed_structs', 'void_functions',
                        'floating_point', 'floating_point_plain', 'helloworld', 'halt',
                        'bits', 'types_2', 'keywords', 'qualifiers', 'static', 'structs_with_pointers',
                        'function_pointers', 'globals', 'multi_arrays', 'alignment', 'types_3', 'memory', 'calc',
                        'variadic', 'symbolic_stdin', 'libfunc', 'linked-list', 'tree']
                ));
                assert.ok(checkTestFileNotEmpty(projectPath));
                await executeCommand(Commands.RunAllTestsAndShowCoverage);
                assert.ok(checkCoverageJson(path.join(projectPath, 'build')));
        }

        async function prepare(compiler: Compiler, verboseTestMode: boolean): Promise<void> {
                await restoreTestDirState(projectPath, compiler);
                await activate(projectPath);
                await Prefs.setVerboseTestMode(verboseTestMode);
                await openFile(vs.Uri.file(openedFile));
        }

        test('[Happy Path]: Generate tests for all the source files with mode=verbose, compiler=Clang', async () => {
                const compiler = Compiler.Clang;
                await prepare(compiler, VERBOSE_TEST_MODE);
                for (const target of UTBotTargetsStorage.instance.targets) {
                        await setTarget(target.name);
                        await executeCommand(Commands.GenerateTestsForProject);
                }
                await checkAll();
        });

        // test('[Happy Path]: Generate tests for all the source files with mode=parametrized, compiler=Clang', async () => {
        //         const compiler = Compiler.Clang;
        //         await prepare(compiler, PARAMETRIZED_TEST_MODE);
        //         await executeCommand(cfg.Commands.GenerateTestsForProject);
        //         await checkAll();
        // });

        // test('[Happy Path]: Generate tests for all the source files with mode=verbose, compiler=GCC', async () => {
        //         const compiler = Compiler.Gcc;
        //         await prepare(compiler, VERBOSE_TEST_MODE);
        //         await executeCommand(cfg.Commands.GenerateTestsForProject);
        //         await checkAll();
        // });

        // test('[Happy Path]: Generate tests for all the source files with mode=parametrized, compiler=GCC', async () => {
        //         const compiler = Compiler.Gcc;
        //         await prepare(compiler, PARAMETRIZED_TEST_MODE);
        //         await executeCommand(cfg.Commands.GenerateTestsForProject);
        //         await checkAll();
        // });

        test('[Happy Path]: `build` folder should be created after the first call', async () => {
                const compiler = Compiler.Clang;
                await restoreTestDirState(projectPath, compiler);
                await activate(projectPath);
                await openFile(vs.Uri.file(openedFile));
                await restoreTestDirState(projectPath, compiler);
                for (const target of UTBotTargetsStorage.instance.targets) {
                        await setTarget(target.name);
                        await executeCommand(Commands.GenerateTestsForProject);
                }
                await checkAll();
        });
});
