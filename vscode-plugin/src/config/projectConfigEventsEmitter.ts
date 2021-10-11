/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import { UTBotEventEmitter } from "../emitter/UTBotEventEmitter";

export class ProjectConfigEventsEmitter {

    private constructor() { }

    private static instance: ProjectConfigEventsEmitter;

    public static getProjectConfigEventsEmitter(): ProjectConfigEventsEmitter {
        if (!this.instance) {
            this.instance = new ProjectConfigEventsEmitter();
        }

        return this.instance;
    }

    readonly onDidImportProject: UTBotEventEmitter<void> = 
        new UTBotEventEmitter<void>();
}
