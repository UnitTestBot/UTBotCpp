/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import { UTBotEventEmitter } from "../emitter/UTBotEventEmitter";

export class WizardEventsEmitter {

    private constructor() {}

    private static instance: WizardEventsEmitter;

    public static getWizardEventsEmitter(): WizardEventsEmitter {
        if (!this.instance) {
            this.instance = new WizardEventsEmitter();
        }

        return this.instance;
    }

    readonly onDidCloseWizardEventEmitter: UTBotEventEmitter<void> = 
        new UTBotEventEmitter();
}
