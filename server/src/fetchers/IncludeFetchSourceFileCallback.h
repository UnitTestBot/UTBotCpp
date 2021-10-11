/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_INCLUDEFETCHSOURCEFILECALLBACK_H
#define UNITTESTBOT_INCLUDEFETCHSOURCEFILECALLBACK_H

#include "Fetcher.h"
#include "clang/Tooling/Tooling.h"

class Fetcher;

class IncludeFetchSourceFileCallback : public clang::tooling::SourceFileCallbacks {
    Fetcher * parent;

public:
    explicit IncludeFetchSourceFileCallback(Fetcher *parent);

public:
    ~IncludeFetchSourceFileCallback() override = default;

protected:
    bool handleBeginSource(clang::CompilerInstance &CI) override;

    void handleEndSource() override;
};


#endif // UNITTESTBOT_INCLUDEFETCHSOURCEFILECALLBACK_H
