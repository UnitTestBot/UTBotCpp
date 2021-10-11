/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "SingleFileParseModeCallback.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PreprocessorOptions.h>

bool SingleFileParseModeCallback::handleBeginSource(clang::CompilerInstance &CI) {
    CI.getPreprocessor().SetSuppressIncludeNotFoundError(true);
    CI.getPreprocessorOpts().SingleFileParseMode = true;
    return true;
}
