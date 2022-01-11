/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as os from 'os';
import * as path from 'path';
import * as vs from 'vscode';
import * as vsUtils from '../utils/vscodeUtils';
import { Prefs } from './prefs';
import * as pathUtils from '../utils/pathUtils';

export class DefaultConfigValues {
    public static readonly DEFAULT_HOST = "127.0.0.1";
    public static readonly DEFAULT_PORT = 2121;

    public static readonly POSSIBLE_BUILD_DIR_NAMES = ['out', 'build'];
    public static readonly POSSIBLE_TEST_DIR_NAMES = ['test'];

    public static getDefaultHost(): string {
        return Prefs.getGlobalHost();
    }

    public static getDefaultPort(): number {
        return parseInt(Prefs.getGlobalPort());
    }

    public static getDefaultRemotePath(): string {
        if (Prefs.isRemoteScenario()) {
            return vsUtils.getRemotePathFromSftpConfig();
        } else {
            return vsUtils.getProjectDirByOpenedFile().fsPath;
        }
    }

    public static async getDefaultBuildDirectoryPath(): Promise<string> {
        // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
        const rootPath = pathUtils.getRootPath();
        let buildDirName = "build";
        if (!rootPath) {
            return buildDirName;
        }
        await vs.workspace.fs.readDirectory(vs.Uri.parse(rootPath))
            .then(resultArray => {
                resultArray.forEach(([name, type]) => {
                    // add only non hidden directories and not a build directory by default
                    if (type === vs.FileType.Directory
                        && DefaultConfigValues.looksLikeBuildDirectory(name)) {
                        buildDirName = name;
                    }
                });
            });
        return buildDirName;
    }

    public static looksLikeBuildDirectory(dirPath: string): boolean {
        return this.looksLike(dirPath, DefaultConfigValues.POSSIBLE_BUILD_DIR_NAMES);
    }

    public static looksLikeTestDirectory(dirPath: string): boolean {
        return this.looksLike(dirPath, this.POSSIBLE_TEST_DIR_NAMES);
    }

    private static looksLike(dirPath: string, patterns: string[]): boolean {
        const name = path.basename(dirPath).toLowerCase();
        let looksLikeOneOfPatterns = false;
        patterns.forEach(possibleName => {
            if (name.includes(possibleName)) {
                looksLikeOneOfPatterns = true;
            }
        });

        return looksLikeOneOfPatterns;
    }
}