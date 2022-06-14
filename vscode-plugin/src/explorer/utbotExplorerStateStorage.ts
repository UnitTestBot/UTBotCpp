import * as vs from 'vscode';

export class UTBotExplorerStateStorage {
    private constructor() {}

    private static instance: UTBotExplorerStateStorage | undefined = undefined;
    private storage: Map<string, vs.TreeItemCollapsibleState> = new Map();

    public static getInstance(): UTBotExplorerStateStorage {
        if (this.instance === undefined) {
            this.instance = new UTBotExplorerStateStorage();
        }

        return this.instance;
    }

    public getSavedState(path: string): vs.TreeItemCollapsibleState | undefined {
        return this.storage.get(path);
    }

    public saveState(path: string, state: vs.TreeItemCollapsibleState): void {
        this.storage.set(path, state);
    }
}
