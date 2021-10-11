/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import { DummyResponse, HeartbeatResponse } from "../proto-ts/testgen_pb";
import { UTBotEventEmitter } from "../emitter/UTBotEventEmitter";

type ErrorMessage = string;

export class ClientEventsEmitter {

    private constructor() {}

    private static instance: ClientEventsEmitter;

    public static getClientEventsEmitter(): ClientEventsEmitter {
        if (!this.instance) {
            this.instance = new ClientEventsEmitter();
        }

        return this.instance;
    }

    readonly onDidHeartbeatFailureEventEmitter: UTBotEventEmitter<ErrorMessage> =
        new UTBotEventEmitter();

    readonly onDidHeartbeatSuccessEventEmitter: UTBotEventEmitter<HeartbeatResponse> =
        new UTBotEventEmitter();

    readonly onDidHandshakeSuccessEventEmitter: UTBotEventEmitter<DummyResponse> =
        new UTBotEventEmitter();

    readonly onDidHandshakeFailureEventEmitter: UTBotEventEmitter<ErrorMessage> =
        new UTBotEventEmitter();

    readonly onDidConnectFirstTimeEventEmitter: UTBotEventEmitter<void> =
        new UTBotEventEmitter();
}
