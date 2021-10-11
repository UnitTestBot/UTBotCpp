/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as fs from "fs";
import * as vs from "vscode";

export class WizardHtmlBuilder {
    
    private content: string;

    constructor(htmlUri: vs.Uri) {
        this.content = fs.readFileSync(htmlUri.fsPath, 'utf8');
    }

    public build(): string {
        return this.content;
    }

    public setVSCodeStyleUri(vscodeStylesUri: vs.Uri): WizardHtmlBuilder {
        return this.setVSCodeStylePath(`${vscodeStylesUri}`);
    }

    public setVSCodeStylePath(vscodeStylesPath: string): WizardHtmlBuilder {
        this.content = this.content.replace('{{vscodeUri}}', vscodeStylesPath);
        return this;
    }

    public setCustomStyleUri(stylesUri: vs.Uri): WizardHtmlBuilder {
        return this.setCustomStylePath(`${stylesUri}`);
    }

    public setCustomStylePath(stylesPath: string): WizardHtmlBuilder {
        this.content = this.content.replace('{{stylesUri}}', stylesPath);
        return this;
    }

    public setScriptUri(scriptUri: vs.Uri): WizardHtmlBuilder {
        return this.setScriptPath(`${scriptUri}`);
    }

    public setScriptPath(scriptPath: string): WizardHtmlBuilder {
        this.content = this.content.replace('{{scriptUri}}', scriptPath);
        return this;
    }

    public setPredictedHost(predictedHost: string): WizardHtmlBuilder {
        this.content = this.content.replace('{{predictedHost}}', predictedHost);
        return this;
    }

    public setPredictedPort(predictedPort: number): WizardHtmlBuilder {
        this.content = this.content.replace('{{predictedPort}}', `${predictedPort}`);
        return this;
    }

    public setPredictedRemotePath(predictedRemotePath: string): WizardHtmlBuilder {
        this.content = this.content.replace('{{predictedRemotePath}}', predictedRemotePath);
        return this;
    }

    public setPredictedBuildDirectory(predictedBuildDirectory: string): WizardHtmlBuilder {
        this.content = this.content.replace('{{predictedBuildDirectory}}', predictedBuildDirectory);
        return this;
    }

    public setPlatform(platform: string): WizardHtmlBuilder {
        this.content = this.content.replace('{{os}}', platform);
        return this;
    }

}