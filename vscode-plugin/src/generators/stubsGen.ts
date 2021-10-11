/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as vs from 'vscode';
import { Client } from '../client/client';
import { utbotUI } from '../interface/utbotUI';
import { ExtensionLogger } from '../logger';
import { StubsResponse } from '../proto-ts/testgen_pb';
import { SourceCode } from '../proto-ts/util_pb';
import { RequestTestsParams } from '../requests/params';
import * as pathUtils from '../utils/pathUtils';
import * as gen from './gen';
import { DummyResponseHandler } from '../responses/responseHandler';
import { Prefs } from '../config/prefs';
const { logger } = ExtensionLogger;


export async function stubProject(client: Client): Promise<void> {
    logger.info('Stub project');
    
    await utbotUI.progresses().withProgress(async (progressKey, token) => {
        utbotUI.progresses().report(progressKey, "UTBot is generating stubs for project...");
        let params: RequestTestsParams;
        try {
            params = gen.getProjectParams(progressKey, token);
        } catch (err) {
            return;
        }
        const responseHandler = new DummyResponseHandler<StubsResponse>();
        const stubs = await client.requestProjectStubs(params, responseHandler);
        const stubsFiles = stubs.getStubsourcesList();
        await handleStubsResponse(stubsFiles);
    });
}


async function handleStubsResponse(stubs: SourceCode[]): Promise<void> {
    if (Prefs.isRemoteScenario()) {
        await Promise.all(stubs.map(async (stub) => {
            const localPath = pathUtils.substituteLocalPath(stub.getFilepath());
            const mockfile = vs.Uri.file(localPath);
            logger.info(`Write mock file ${stub.getFilepath()} to ${localPath}`);
            await vs.workspace.fs.writeFile(mockfile, Buffer.from(stub.getCode()));
        }));
    }
}