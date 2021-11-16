/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as path from 'path';
import * as vs from "vscode";
import * as messages from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import { DataLoader } from "../dataloader/dataLoader";
import { StateDecorationTypes } from "../interface/stateDecorationTypes";
import { FileCoverageSimplified, SourceLine } from "../proto-ts/testgen_pb";
import * as pathUtils from '../utils/pathUtils';
import { Visualizer } from "./visualizer";

type CoverageObject = { [index: string]: FileCoverage };

export class CoverageVisualizer implements Visualizer, DataLoader<FileCoverageSimplified, CoverageObject> {
    hidden: boolean;
    private decorations: StateDecorationTypes;
    private coverages: CoverageObject = {};
    private editors: Set<vs.TextEditor> = new Set();

    constructor(context: vs.ExtensionContext) {
        this.decorations = new StateDecorationTypes(context);
        this.hidden = false;
        this.subscribe();
    }

    subscribe(): void {
        vs.window.onDidChangeActiveTextEditor(async (editor) => {
            if (editor) {
                await this.display(editor);
            }
        });
        vs.workspace.onDidDeleteFiles((event) => {
            event.files.forEach(fileUri => {
                this.clearCoverageByFileName(fileUri.fsPath);
            });
        });
        vs.workspace.onDidChangeTextDocument((event) => {
            this.editors.forEach((editor) => {
                if (editor.document.uri === event.document.uri) {
                    this.hide(editor);
                    this.clearEditorCoverage(editor);
                }
            });
        });
    }

    public getLoadedData(): CoverageObject {
        return this.coverages;
    }

    async loadData(simplifiedFileCoverages: Array<FileCoverageSimplified> | undefined = undefined): Promise<void> {
        for (const fn in this.coverages) {
            this.coverages[fn].update();
        }
        this.checkLines(simplifiedFileCoverages);
        if (simplifiedFileCoverages !== undefined) {
            this.getLines(simplifiedFileCoverages);
        }
    }

    public async display(editor: vs.TextEditor): Promise<void> {
        if (this.hidden) {
            return;
        }
        let fileName = editor.document.fileName;
        if (Prefs.isRemoteScenario()) {
            let trimmed = '';
            if (process.platform === 'win32') {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                trimmed = fileName.split(path.win32.sep).pop()!;
            } else {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                trimmed = fileName.split(path.posix.sep).pop()!;
            }
            // TODO: rm this dirty hack
            for (const fn in this.coverages) {
                if (fn.endsWith(trimmed)) {
                    fileName = fn;
                    break;
                }
            }
        }
        const cov = this.coverages[fileName];
        if (cov === undefined) {
            return;
        }

        this.hide(editor);
        this.editors.add(editor);

        editor.setDecorations(this.decorations.noCoverage, FileCoverage.setToRanges(cov.noCoverageLines));
        editor.setDecorations(this.decorations.partialCoverage, FileCoverage.setToRanges(cov.partialCoverageLines));
        editor.setDecorations(this.decorations.oldCoverage, FileCoverage.setToRanges(cov.oldCoverageLines));
        editor.setDecorations(this.decorations.fullCoverage, FileCoverage.setToRanges(cov.fullCoverageLines));
    }

    public hideAll(): void {
        this.hidden = true;
        this.coverages = {};
        this.editors.forEach(editor => {
            this.hide(editor);
        });
    }

    public hide(editor: vs.TextEditor): void {
        editor.setDecorations(this.decorations.fullCoverage, []);
        editor.setDecorations(this.decorations.oldCoverage, []);
        editor.setDecorations(this.decorations.partialCoverage, []);
        editor.setDecorations(this.decorations.noCoverage, []);
        this.editors.delete(editor);
    }

    public clearData(): void {
        this.coverages = {};
    }

    public clearEditorCoverage(editor: vs.TextEditor): void {
        this.clearCoverageByFileName(editor.document.fileName, false);
    }

    public clearCoverageByFileName(filename: string, canBeFolder: boolean = true): void {
        let serverFileName = filename;
        if (Prefs.isRemoteScenario()) {
            serverFileName = pathUtils.substituteRemotePath(serverFileName);
        }

        for (const fn in this.coverages) {
            if (fn === serverFileName || canBeFolder && fn.startsWith(serverFileName)) {
                this.coverages[fn] = new FileCoverage();
            }
        }
    }

    checkLines(simplifiedFileCoverages: Array<FileCoverageSimplified> | undefined = undefined): void {
        if (simplifiedFileCoverages === undefined ||
            simplifiedFileCoverages.filter(f => { return f.getFullcoveragelinesList().length > 0; }).length === 0) {
            messages.showInfoMessage(`There are no covered lines. Maybe some of the tests were aborted.`);
        }
    }

    getLine(element: SourceLine): number | undefined {
        const line = element.getLine();
        return line;
    }

    getLines(implifiedFileCoverages: Array<FileCoverageSimplified>): void {
        for (const simplifiedFileCoverage of implifiedFileCoverages) {
            let filepath = simplifiedFileCoverage.getFilepath();
            if (process.platform === 'win32') {
                const systemPrefix = filepath.split(':')[0];
                filepath = systemPrefix + ':' + filepath.split(':').slice(1).join(':');
                filepath = filepath.slice(0, filepath.length - 1);
            }
            if (this.coverages[filepath] === undefined) {
                this.coverages[filepath] = new FileCoverage();
            }
            const cov = this.coverages[filepath];
          
            simplifiedFileCoverage.getFullcoveragelinesList().forEach(element => {
                const line = this.getLine(element);
                if(line !== undefined) {
                    cov.fullCoverageLines.add(line);
                }
            });
            simplifiedFileCoverage.getPartialcoveragelinesList().forEach(element => {
                const line = this.getLine(element);
                if(line !== undefined) {
                    cov.partialCoverageLines.add(line);
                }
            });
            simplifiedFileCoverage.getNocoveragelinesList().forEach(element => {
                const line = this.getLine(element);
                if(line !== undefined) {
                    cov.noCoverageLines.add(line);
                }
            });
        }
    }
}

class FileCoverage {
    fullCoverageLines: Set<number> = new Set<number>();
    oldCoverageLines: Set<number> = new Set<number>();
    partialCoverageLines: Set<number> = new Set<number>();
    noCoverageLines: Set<number> = new Set<number>();

    noCoverageLinesBorders: Set<number> = new Set<number>();

    public static setToRanges(lines: Set<number>): vs.Range[] {
        const ranges: vs.Range[] = [];
        lines.forEach(line => {
            ranges.push(new vs.Range(line, 0, line, 0));
        });
        return ranges;
    }

    public update(): void {
        this.fullCoverageLines.forEach(line => this.oldCoverageLines.add(line));
        this.fullCoverageLines = new Set();
        this.partialCoverageLines = new Set();
        this.noCoverageLines = new Set();
        this.noCoverageLinesBorders = new Set();
    }
}