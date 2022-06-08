import { UTBotProjectTarget } from "./UTBotProjectTarget";

export type Path = string;

export class UTBotTargetsStorage {

    private constructor() { }

    private static _instance: UTBotTargetsStorage | undefined = undefined;

    private _targets: Array<UTBotProjectTarget> = [];

    public get targets(): Array<UTBotProjectTarget> {
        return this._targets;
    }
    public set targets(value: Array<UTBotProjectTarget>) {
        this._targets = value;
    }

    private _primaryTargetPath: string | undefined = undefined;
    public get primaryTargetPath(): string | undefined {
        return this._primaryTargetPath;
    }
    public set primaryTargetPath(value: string | undefined) {
        this._primaryTargetPath = value;
    }

    public static get instance(): UTBotTargetsStorage {
        if (this._instance === undefined) {
            this._instance = new UTBotTargetsStorage();
        }
        return this._instance;
    }
}
