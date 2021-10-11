/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as vs from 'vscode';
import { IconPath, IconPaths } from './iconPaths';

export class StateDecorationTypes {

	readonly passed: vs.TextEditorDecorationType;
	readonly failed: vs.TextEditorDecorationType;
	readonly dead: vs.TextEditorDecorationType;
	readonly fullCoverage: vs.TextEditorDecorationType;
	readonly oldCoverage: vs.TextEditorDecorationType;
	readonly partialCoverage: vs.TextEditorDecorationType;
	readonly noCoverage: vs.TextEditorDecorationType;
	readonly all: vs.TextEditorDecorationType[];

	constructor(context: vs.ExtensionContext) {
		const iconPaths: IconPaths = new IconPaths(context);
		this.passed = toDecorationType(iconPaths.passed);
		this.failed = toDecorationType(iconPaths.failed);
		this.dead = toDecorationType(iconPaths.errored);
		this.fullCoverage = toDecorationType(iconPaths.fullCoverage);
		this.oldCoverage = toDecorationType(iconPaths.oldCoverage);
		this.partialCoverage = toDecorationType(iconPaths.partialCoverage);
		this.noCoverage = toDecorationType(iconPaths.noCoverage);

		this.all = [
			this.passed, this.failed, this.dead, 
			this.fullCoverage, this.oldCoverage, this.partialCoverage, this.noCoverage
		];

		for (const decorationType of this.all) {
			context.subscriptions.push(decorationType);
		}
	}
}

function toDecorationType(iconPath: IconPath): vs.TextEditorDecorationType {
	return vs.window.createTextEditorDecorationType(toDecorationRenderOptions(iconPath));
}

function toDecorationRenderOptions(iconPath: IconPath): vs.DecorationRenderOptions {
	if (typeof iconPath === 'string') {
		return { gutterIconPath: iconPath };
	} else {
		return {
			dark: { gutterIconPath: iconPath.dark },
			light: { gutterIconPath: iconPath.light }
		};
	}
}