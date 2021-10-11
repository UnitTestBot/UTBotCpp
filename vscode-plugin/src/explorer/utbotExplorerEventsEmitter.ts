/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import { UTBotEventEmitter } from "../emitter/UTBotEventEmitter";
import { } from "emittery";
import { UTBotProjectTargetsList } from "./UTBotProjectTarget";

export class UtbotExplorerEventsEmitter {

    private constructor() {}

    private static instance: UtbotExplorerEventsEmitter;

    public static getUbotExplorerEventsEmitter(): UtbotExplorerEventsEmitter {
        if (!this.instance) {
            this.instance = new UtbotExplorerEventsEmitter();
        }

        return this.instance;
    }

    readonly onDidRequestSetDefaultSourceFoldersEventsEmmiter: UTBotEventEmitter<void> =
        new UTBotEventEmitter();

    readonly onDidRequestTargetsEmitter: UTBotEventEmitter<void> = 
        new UTBotEventEmitter();
        
    readonly onDidAddTargetsEventEmitter: UTBotEventEmitter<UTBotProjectTargetsList> =
        new UTBotEventEmitter();
}
