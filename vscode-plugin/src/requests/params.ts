/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as vs from 'vscode';
import { GTestInfo } from '../cache/testsCache';
import { Prefs } from '../config/prefs';
import { utbotUI } from '../interface/utbotUI';

export type Progress = vs.Progress<{message?: string; increment?: number}>;

export class RequestTestsParams {
    readonly synchronizeCode: boolean;
    
    constructor(
        readonly projectPath: string,
        readonly buildDirRelativePath: string,
        readonly projectName: string,
        readonly sourcePaths: string[],
        readonly targetPath: string,
        readonly progressKey: utbotUI.ProgressKey,
        readonly token: vs.CancellationToken) {
        this.synchronizeCode = Prefs.isRemoteScenario();
    }
}

export class RequestCoverageAndResultParams {
    constructor(readonly projectPath: string,
                readonly buildDirRelativePath: string,
                readonly testInfo: GTestInfo | undefined,
                readonly progressKey: utbotUI.ProgressKey,
                readonly cancellationToken: vs.CancellationToken) {}
}

export class RequestTestResultsParams {
    constructor(readonly projectPath: string,
                readonly buildDirRelativePath: string,
                readonly execPath: string,
                readonly fileName: string,
                readonly testName: string,
                readonly progressKey: utbotUI.ProgressKey,
                readonly cancellationToken: vs.CancellationToken
    ) {}
}
