/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as fs from 'fs';
import * as vs from 'vscode';
import * as pathUtils from '../utils/pathUtils';

export function getProjectDirByOpenedFile(): vs.Uri {
    const openedFile = getTextEditor()?.document.uri.fsPath;
    let projectDir: string | undefined = undefined;
    if (!vs.workspace.workspaceFolders) {
        throw new Error("No opened workspace folders.");
    }

    if (!openedFile) {
        projectDir = pathUtils.getRootPath();
    } else {
        vs.workspace.workspaceFolders.forEach(folder => {
            if (openedFile.startsWith(folder.uri.fsPath)) {
                projectDir = folder.uri.fsPath;
            }
        });
    }
    if (projectDir === undefined) {
        projectDir = pathUtils.getRootPath();
    }
    // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
    return vs.Uri.file(projectDir!);
}

export function getTextEditor(): vs.TextEditor | undefined {
    if (vs.window.visibleTextEditors.length === 1 && vs.window.activeTextEditor) {
        // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
        return vs.window.activeTextEditor!;
    }
    if (vs.window.visibleTextEditors.length > 1) {
        let textEditor: vs.TextEditor | undefined = undefined;
        vs.window.visibleTextEditors.forEach(editor => {
            if (!textEditor && fs.existsSync(editor.document.uri.fsPath)) {
                textEditor = editor;
            }
        });
        if (textEditor) {
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            return textEditor!;
        }
    }
    return undefined;
}
