/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as vs from "vscode";
import { GTestInfo } from "../cache/testsCache";
import { Client } from "../client/client";
import * as messages from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import { DummyResponseHandler } from "../responses/responseHandler";
import { utbotUI } from "../interface/utbotUI";
import { ExtensionLogger } from '../logger';
import { CoverageAndResultsResponse } from "../proto-ts/testgen_pb";
import { RequestCoverageAndResultParams } from "../requests/params";
import * as vsUtils from '../utils/vscodeUtils';
import { CoverageVisualizer } from "../visualizers/coverageVisualizer";
import { TestResultsVisualizer } from "../visualizers/testResultsVisualizer";
const { logger } = ExtensionLogger;

export class TestsRunner {
    public readonly coverageVizualizer: CoverageVisualizer;
    public readonly testResultsVizualizer: TestResultsVisualizer;


    constructor(context: vs.ExtensionContext) {
        this.coverageVizualizer = new CoverageVisualizer(context);
        this.testResultsVizualizer = new TestResultsVisualizer(context);
    }


    public async run(client: Client, testInfo: GTestInfo | undefined): Promise<void> {
        if (client.isConnectionNotEstablished() || client.newClient) {
            try {
                await client.heartbeat();
            } catch(error) {
                logger.error(`Heartbeat error: ${error}`);
            }
        }
        try {
            const buildDirPath = Prefs.getBuildDirPath();
            await utbotUI.progresses().withProgress(async (progressKey, cancellationToken) => {
                const params = new RequestCoverageAndResultParams(
                    buildDirPath[0],
                    buildDirPath[1],
                    testInfo,
                    progressKey,
                    cancellationToken
                );
                const responseHandler = new DummyResponseHandler<CoverageAndResultsResponse>();
                const coverageAndResults =
                    await client.requestCoverageAndResults(params, responseHandler);
                const errorMessage = coverageAndResults.getErrormessage();
                if (errorMessage.length > 0) {
                    messages.showErrorMessage(errorMessage);
                    utbotUI.channels().outputClientLogChannel.show(true);
                }
                logger.debug(`Test results quantity: ${coverageAndResults.getTestrunresultsList().length}`);
                await this.loadAllVisualizers(coverageAndResults).then(async () => {
                    await this.setHidden(false);
                    const editor = vsUtils.getTextEditor();
                    if (editor) {
                        await this.displayAllVisualizers(editor);
                    }
                });
            });
        } catch (err) {
            logger.error(err);
        }
    }

    public hideTestResultsAndCoverage(): void {
        this.coverageVizualizer.hideAll();
        this.testResultsVizualizer.hideAll();
    }

    private async setHidden(hidden: boolean): Promise<void> {
        this.coverageVizualizer.hidden = hidden;
        this.testResultsVizualizer.hidden = hidden;
    }

    private async loadAllVisualizers(coverageAndResults: CoverageAndResultsResponse): Promise<void> {
        await this.coverageVizualizer.loadData(coverageAndResults.getCoveragesList());
        await this.testResultsVizualizer.loadData(coverageAndResults.getTestrunresultsList());
    }

    private async displayAllVisualizers(editor: vs.TextEditor): Promise<void> {
        await this.coverageVizualizer.display(editor);
        await this.testResultsVizualizer.display(editor);
    }
}