/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as path from 'path';
import * as vs from "vscode";
import * as messages from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import { DataLoader } from "../dataloader/dataLoader";
import { StateDecorationTypes } from "../interface/stateDecorationTypes";
import { FileCoverageSimplified, SourceRange } from "../proto-ts/testgen_pb";
import * as pathUtils from '../utils/pathUtils';
import { Visualizer } from "./visualizer";

type CoverageObject = { [index: string]: FileCoverage };

export class CoverageVisualizer implements Visualizer, DataLoader<FileCoverageSimplified, CoverageObject> {
    hidden: boolean;
    private decorations: StateDecorationTypes;
    private coverages: CoverageObject = {};
    private editors: Set<vs.TextEditor> = new Set();

    rangeFor = (start: number, end: number): number[] =>
        Array.from({ length: (end - start + 1) }, (v, k) => k + start);

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
        this.checkRanges(simplifiedFileCoverages);
        if (simplifiedFileCoverages !== undefined) {
            this.getRanges(simplifiedFileCoverages);
            this.getLines();
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

    checkRanges(simplifiedFileCoverages: Array<FileCoverageSimplified> | undefined = undefined): void {
        if (simplifiedFileCoverages === undefined ||
            simplifiedFileCoverages.filter(f => { return f.getCoveredrangesList().length > 0; }).length === 0) {
            messages.showInfoMessage(`There are no covered lines. Maybe some of the tests were aborted.`);

        }
    }

    getRange(element: SourceRange): vs.Range | undefined {
        const startLine = element.getStart()?.getLine();
        const startCharacter = element.getStart()?.getCharacter();
        const endLine = element.getEnd()?.getLine();
        const endCharacter = element.getEnd()?.getCharacter();
        if (startLine !== undefined && startCharacter !== undefined &&
            endLine !== undefined && endCharacter !== undefined) {
            return new vs.Range(startLine, startCharacter, endLine, endCharacter);
        }
    }

    getRanges(implifiedFileCoverages: Array<FileCoverageSimplified>): void {
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
            simplifiedFileCoverage.getCoveredrangesList().forEach(element => {
                const range = this.getRange(element);
                if (range !== undefined) {
                    cov.coveredRanges.push(range);
                }
            });
            simplifiedFileCoverage.getUncoveredrangesList().forEach(element => {
                const range = this.getRange(element);
                if (range !== undefined) {
                    cov.noCoveredRanges.push(range);
                }
            });
        }
    }

    checkLineForPartial(line: number, filename: string): void {
        if (this.coverages[filename].noCoverageLinesBorders.has(line)) {
            this.coverages[filename].partialCoverageLines.add(line);
            this.coverages[filename].noCoverageLines.delete(line);
        } else {
            this.coverages[filename].fullCoverageLines.add(line);
        }
    }

    getLines(): void {
        for (const filename in this.coverages) {
            const cov = this.coverages[filename];
            cov.noCoveredRanges.forEach(range => {
                cov.noCoverageLinesBorders.add(range.start.line);
                cov.noCoverageLinesBorders.add(range.end.line);
                this.rangeFor(range.start.line, range.end.line).forEach(line => {
                    cov.noCoverageLines.add(line);
                });
            });
            cov.coveredRanges.forEach(range => {
                this.checkLineForPartial(range.start.line, filename);
                this.checkLineForPartial(range.end.line, filename);
                this.rangeFor(range.start.line + 1, range.end.line - 1).forEach(line => {
                    if (!cov.noCoverageLines.has(line)) {
                        cov.fullCoverageLines.add(line);
                    }
                });
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

    noCoveredRanges: vs.Range[] = [];
    coveredRanges: vs.Range[] = [];

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
        this.noCoveredRanges = [];
        this.coveredRanges = [];
    }
}