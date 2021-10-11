/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import { exec } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as vs from 'vscode';
import { Commands } from '../config/commands';
import { UTBotTargetsStorage } from '../explorer/utbotTargetsStorage';
import * as pathUtils from '../utils/pathUtils';
import { TestLogger } from "./testLogger";

export enum Compiler { Clang, Gcc }

export async function activate(projectPath: string): Promise<void> {
    // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
    const ext = vs.extensions.getExtension('huawei.unittestbot')!;
    await ext.activate().then(api => api.setProjectPath(projectPath));
}

export const VERBOSE_TEST_MODE = true;

export const PARAMETRIZED_TEST_MODE = false;

export async function openFile(docUri: vs.Uri, linePosition: number | undefined = undefined): Promise<void> {
    const doc = await vs.workspace.openTextDocument(docUri);
    const editor = await vs.window.showTextDocument(doc);
    if (linePosition !== undefined) {
        const position = editor.selection.active;
        const newPosition = position.with(linePosition, 0);
        // noinspection UnnecessaryLocalVariableJS
        const newSelection = new vs.Selection(newPosition, newPosition);
        editor.selection = newSelection;
    }
}

async function deleteDir(dirPath: string): Promise<void> {
    return new Promise((resolve, reject) => {
        exec(`rm -rf ${dirPath}`, (err, _stdout, _stderr) => {
            if (err) {
                reject(err);
            }
            resolve();
        });
    });
}

export async function clearTestDir(dirPath: string): Promise<void> {
    await deleteDir(pathUtils.fsJoin(dirPath, 'build'));
    await deleteDir(pathUtils.fsJoin(dirPath, 'tests'));
}

export async function execCommand(command: string, workdir: string): Promise<void> {
    return new Promise((resolve, reject) => {
        exec(`cd ${workdir} && ${command}`,
            (err, _stdout, _stderr) => {
                if (err) {
                    reject(err);
                }
                resolve();
            });
    });
}

export async function execBash(scriptPath: string, scriptName: string): Promise<void> {
    const fullPath = pathUtils.fsJoin(scriptPath, scriptName);
    return await execCommand(`bash ${fullPath}`, scriptPath);
}

export async function restoreTestDirState(dirPath: string, compiler: Compiler): Promise<void> {
    await clearTestDir(dirPath);
    await rebuildProject(dirPath, compiler);
}

function getBuildCommand(compiler: Compiler): string {
    const bear = "/utbot_distr/bear/bin/bear";
    const cmake = "/utbot_distr/install/bin/cmake";
    const clang = "/utbot_distr/install/bin/clang";
    const clangpp = "/utbot_distr/install/bin/clang++";

    switch (compiler) {
        case Compiler.Clang:
            return 'export CC=' + clang + ' && export CXX=' + clangpp + ' && ' +
                'mkdir -p build && cd build && ' + cmake + ' .. && ' + bear + ' make -j8';
        case Compiler.Gcc:
            return 'export C_INCLUDE_PATH="" && export CC=gcc && export CXX=g++ && ' +
                'mkdir -p build && cd build && ' + cmake + ' .. && ' + bear + ' make -j8';
        default:
            // eslint-disable-next-line
            throw new Error("Compiler could not be identified " + compiler);
    }
}

export async function rebuildProject(dirPath: string, compiler: Compiler): Promise<void> {
    TestLogger.getLogger().info('Recompiling the project [%s] with [%s] compiler', dirPath, Compiler[compiler]);
    await execCommand(getBuildCommand(compiler), dirPath);
}

export function checkDirectoryWithTestsExists(dirPath: string): boolean {
    const testDirPath = pathUtils.fsJoin(dirPath, 'tests');
    printDirectoryContents(testDirPath);
    TestLogger.getLogger().info('Checking for directory [%s] exists at time [%s]', testDirPath, new Date());
    return fs.existsSync(testDirPath);
}

export function printDirectoryContents(dir: string): void {
    TestLogger.getLogger().trace('Printing [%s] folder content', dir);
    fs.readdir(dir, function (err, _files) {
        if (err) {
            return TestLogger.getLogger().error('Unable to scan directory: ' + err);
        }
    });
}

function walk(dir: string): string[] {
    let files: string[] = [];
    fs.readdirSync(dir).forEach((entry) => {
        entry = path.resolve(dir, entry);
        const stat = fs.lstatSync(entry);
        if (stat.isDirectory()) {
            files = files.concat(walk(entry));
        } else {
            files.push(entry);
        }
    });
    return files;
}

export function checkTestFilesGenerated(dirPath: string, srcFiles: string[]): boolean {
    const testDirPath = pathUtils.fsJoin(dirPath, 'tests');
    TestLogger.getLogger().trace('Scanning folder [%s] for tests', testDirPath);
    TestLogger.getLogger().trace('Src files are [%s]', srcFiles);
    let checked = true;
    const srcFilesUsedMap = new Map<string, boolean>(srcFiles.map(file => [file, false]));
    walk(testDirPath).forEach(file => {
        const testFileName = path.parse(file).name;
        if (testFileName.search(/_stub$/) < 0) {
            const fileName = checkForFirstMatch(testFileName, [/_test_error$/, /_test$/]);
            if (!srcFilesUsedMap.has(fileName)) {
                TestLogger.getLogger().error('Unable to find a corresponding source file for test: [%s]', testFileName);
                checked = false;
            } else {
                srcFilesUsedMap.set(fileName, true);
            }
        }
    });
    if (!Array.from(srcFilesUsedMap.values()).every(Boolean)) {
        const noTestedSources = Array.from(srcFilesUsedMap.entries()).filter(([_file, used]) => !used).map(([file, _]) => file);
        TestLogger.getLogger().error('Unable to find tests files for corresponding sources: [%s]', noTestedSources);
        checked = false;
    }
    return checked;
}

function checkForFirstMatch(target: string, patterns: RegExp[]): string {
    let ret = '';
    patterns.every(function (pattern, _index) {
        const found = target.search(pattern) >= 0;
        if (found) {
            ret = target.replace(pattern, '');
            TestLogger.getLogger().trace('target=[%s], result=[%s]', target, target.replace(pattern, ''));
        }
        return true;
    });
    return ret;
}

export function checkTestFileNotEmpty(dirPath: string): boolean {
    const testDirPath = pathUtils.fsJoin(dirPath, 'tests');
    walk(testDirPath).forEach(file => {
        TestLogger.getLogger().trace('reading file [%s]', file);
        if (fs.readFileSync(file).length === 0) {
            TestLogger.getLogger().error('Test file [%s] is empty', file);
            return false;
        }
    });
    return true;
}

export function checkCoverageJson(buildDirPath: string): boolean {
    printDirectoryContents(buildDirPath);
    return true;
    //TODO check client info instead of artifacts
    // return clangCoverageExists(buildDirPath) || gccCoverageExists(buildDirPath);
}

async function setTargetPath(targetPath: string): Promise<void> {
    await vs.commands.executeCommand(Commands.AddUTBotTargetPath, targetPath);
}

export async function setTarget(targetName: string): Promise<void> {
    await vs.commands.executeCommand(Commands.RefreshUTBotTargetsView);
    const targets = UTBotTargetsStorage.instance.targets;
    const targetPath = targets.find(target => target.name.endsWith(`lib${targetName}.a`) || target.name === targetName);
    if (!targetPath) {
        const msg = String(`Target with name ${targetName} not found.\nAll target names: ${targets.map(target => target.name)}.`);
        TestLogger.getLogger().error(msg);
        return Promise.reject<void>(msg);
    } else {
        return setTargetPath(targetPath.path);
    }
}