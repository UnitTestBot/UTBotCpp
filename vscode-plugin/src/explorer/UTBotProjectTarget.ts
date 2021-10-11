/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import { ProjectTarget } from "../proto-ts/testgen_pb";

export class UTBotProjectTarget {
    constructor(
        public readonly name: string,
        public readonly path: string,
        public readonly description: string
    ) {

    }

    static fromProjectTarget(
        projectTarget: ProjectTarget
    ): UTBotProjectTarget {
        return new UTBotProjectTarget(
            projectTarget.getName(),
            projectTarget.getPath(),
            projectTarget.getDescription()
        );
    }

    public static compareFn(a: UTBotProjectTarget, b: UTBotProjectTarget): number {
        if (a.isAuto()) {
            return -1;
        } else if (b.isAuto()) {
            return 1;
        } else {
            return a.name.localeCompare(b.name);
        }
    }

    public isAuto(): boolean {
        return this.name === "UTBot: auto";
    }
}

export class UTBotProjectTargetsList {
    constructor(
        public readonly targets: Array<UTBotProjectTarget>,
        public readonly priorityTarget: UTBotProjectTarget | undefined = undefined
    ) {

    }
}