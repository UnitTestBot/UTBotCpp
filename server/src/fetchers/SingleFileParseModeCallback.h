/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SINGLEFILEPARSEMODECALLBACK_H
#define UNITTESTBOT_SINGLEFILEPARSEMODECALLBACK_H

#include <clang/Tooling/Tooling.h>

class SingleFileParseModeCallback : public clang::tooling::SourceFileCallbacks {
public:
    bool handleBeginSource(clang::CompilerInstance &CI) override;
};


#endif // UNITTESTBOT_SINGLEFILEPARSEMODECALLBACK_H
