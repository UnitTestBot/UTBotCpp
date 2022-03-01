/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

vscode_test = require('@vscode/test-electron');
vscode_version = process.argv[2];

const download = async function() {
    vscode_test.downloadAndUnzipVSCode(vscode_version)
    .then(res => console.log(res))
    .catch(e => console.log(e));
}

download();