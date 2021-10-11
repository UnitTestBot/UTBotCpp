/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as vs from "vscode";

export interface Visualizer {

    /**
     * Visualize some data in specific editor.
     * @param editor - editor to display data in. 
     */
    display(editor: vs.TextEditor): Promise<void>;
    
    /**
     * Hide visualization of data in specific editor.
     * @param editor - editor to hide visualization in.
     */
    hide(editor: vs.TextEditor): void;

    /**
     * forall editor in editors: hide(editor).
     */
    hideAll(): void;
}