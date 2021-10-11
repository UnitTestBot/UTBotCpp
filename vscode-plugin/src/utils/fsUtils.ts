/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

/* eslint-disable no-inner-declarations */
import * as fs from 'fs';
import * as vs from 'vscode';
import * as pathLib from 'path';

/**
 * Namespace with utils that operate with files of the system. 
 * VSCode Remote scenario: server paths
 * SFTP scenario: local path 
 */
export namespace FSUtils {
    export function isDirectory(path: string): boolean {
        return fs.statSync(vs.Uri.file(path).fsPath).isDirectory();
    }

    export function isFile(path: string): boolean {
        return fs.statSync(vs.Uri.file(path).fsPath).isFile();
    }

    export function getDirectories(path: string): string[] {
        const parsedPath = vs.Uri.file(path).fsPath;
        return fs.readdirSync(parsedPath).map(name => pathLib.join(parsedPath, name)).filter(isDirectory);
    }

    export function getFiles(path: string): string[] {
        const parsedPath = vs.Uri.file(path).fsPath;
        return fs.readdirSync(parsedPath).map(name => pathLib.join(parsedPath, name)).filter(isFile);
    }


    export function getFilesRecursive(path: string): string[] {
        const dirs = getDirectories(path);
        const files = dirs
            .map(dir => getFilesRecursive(dir)) // go through each directory
            .reduce((a, b) => a.concat(b), []);    // map returns a 2d array (array of file arrays) so flatten
        return files.concat(getFiles(path));
    }

    export function getDirectoriesRecursive(path: string): string[] {
        const dirs = getDirectories(path);
        const result: string[] = [];
        result.push(...dirs);
        dirs.forEach(dir => {
            result.push(...getDirectoriesRecursive(dir));
        });

        return result;
    }
}
