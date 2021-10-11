/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SOURCEFILECHAINEDCALLBACKS_H
#define UNITTESTBOT_SOURCEFILECHAINEDCALLBACKS_H

#include <clang/Tooling/Tooling.h>

#include <memory>
#include <vector>

class SourceFileChainedCallbacks : public clang::tooling::SourceFileCallbacks {
private:
    std::vector<std::unique_ptr<SourceFileCallbacks>> callbacks{};

public:
    SourceFileChainedCallbacks() = default;

    explicit SourceFileChainedCallbacks(
        std::vector<std::unique_ptr<clang::tooling::SourceFileCallbacks>> callbacks);

    void add(std::unique_ptr<clang::tooling::SourceFileCallbacks> callback);

    bool handleBeginSource(clang::CompilerInstance &CI) override;
};


#endif // UNITTESTBOT_SOURCEFILECHAINEDCALLBACKS_H
