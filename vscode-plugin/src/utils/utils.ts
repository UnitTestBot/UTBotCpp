import * as vs from 'vscode';
import { Commands } from '../config/commands';
import { Extension } from '../config/config';
import * as os from 'os';

export function getExtensionCommands(): any[] {
    const pkgJSON = vs.extensions.getExtension(Extension.UTBotExtensionID)?.packageJSON;
    if (!pkgJSON.contributes || !pkgJSON.contributes.commands) {
        return [];
    }

    return vs.extensions
        .getExtension(Extension.UTBotExtensionID)?.packageJSON.contributes.commands
        .filter((x: any) => x.command !== Commands.ShowAllCommands);
}

export class Wrapper<T> {
    private _value: T;

    constructor(value: T) {
        this._value = value;
    }

    get value(): T {
        return this._value;
    }

    set value(value: T) {
        this._value = value;
    }
}

export function registerCommand(command: string, callback: (...args: any[]) => void | PromiseLike<void>, thisArg?: any): vs.Disposable {
    const promiseCallback = (...args: any[]): Promise<void> => Promise.resolve<void>(callback(...args));
    return vs.commands.registerCommand(command, promiseCallback, thisArg);
}

export function registerTextEditorCommand(command: string, callback: (textEditor: vs.TextEditor) => void | PromiseLike<void>, thisArg?: any): vs.Disposable {
    const promiseCallback = (): Promise<void> => {
        const textEditor = vs.window.activeTextEditor;
        if (!textEditor) {
            return Promise.reject("There is no active editor. Are all editors closed?");
        }
        return Promise.resolve<void>(callback(textEditor));
    };
    return registerCommand(command, promiseCallback, thisArg);
}

export async function executeCommand(command: string, ...rest: any[]): Promise<void> {
    return Promise.resolve(vs.commands.executeCommand<Promise<void>>(command, ...rest));
}

export function isWin32(): boolean {
    return os.platform() === 'win32';
}
