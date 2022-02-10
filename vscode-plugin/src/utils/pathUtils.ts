/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as path from 'path';
import * as vs from 'vscode';
import { Prefs } from '../config/prefs';
import * as vsUtils from './vscodeUtils';
import {isWin32} from './utils';

export function substituteRemotePath(localFilePath: string): string {
    if (!Prefs.isRemoteScenario()) {
        return localFilePath;
    }
    const remoteProjectDirPath = Prefs.getRemoteRoot();
    const projectDirName = path.basename(vsUtils.getProjectDirByOpenedFile().fsPath);
    let remoteFilePath = fsJoin(remoteProjectDirPath,
        localFilePath.slice(localFilePath.indexOf(projectDirName) + projectDirName.length));
    if (isWin32()) {
        remoteFilePath = remoteFilePath.replace(/\\/g, '/');
    }
    remoteFilePath = normalizeRawPosixPath(remoteFilePath);
    return remoteFilePath;
}

export function substituteLocalPath(remoteFilePath: string): string {
    if (!Prefs.isRemoteScenario()) {
        return remoteFilePath;
    }
    const root = vsUtils.getProjectDirByOpenedFile();
    const remoteRoot = Prefs.getRemoteRoot();
    const projectDirName = path.basename(remoteRoot);
    const relative = remoteFilePath.slice(remoteFilePath.indexOf(projectDirName) + projectDirName.length);
    const localFilePath = fsJoin(root.fsPath, relative);
    if (isWin32()) {
        return path.win32.normalize(localFilePath);
    }
    return localFilePath;
}

export function normalizeRawPosixPath(posixPath: string): string {
    posixPath = path.posix.normalize(posixPath);
    posixPath = posixPath.replace(/\/+$/, '');
    return posixPath;
}

export function getRealFilePath(editorPath: string, optionalClickedPath: vs.Uri | undefined): string {
    if (optionalClickedPath) {
        return optionalClickedPath?.fsPath;
    }
    return editorPath;
}

export function getRootPath(): string | undefined {
    return vs.workspace.workspaceFolders?.[0].uri.path;
}

export function fsJoin(...paths: string[]): string {
    if (isWin32()) {
        return path.win32.join(...paths);
    } else {
        return path.posix.join(...paths);
    }
}