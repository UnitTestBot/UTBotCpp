/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SIMPLEFRONTENDACTIONFACTORY_H
#define UNITTESTBOT_SIMPLEFRONTENDACTIONFACTORY_H


#include <clang/Tooling/Tooling.h>

template <typename T, typename Parent>
class SimpleFrontendActionFactory : public clang::tooling::FrontendActionFactory {
private:
    Parent const *const parent;

public:
    explicit SimpleFrontendActionFactory(const Parent *parent) : parent(parent) {}

    ~SimpleFrontendActionFactory() override = default;

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<T>(parent);
    }
};


#endif //UNITTESTBOT_SIMPLEFRONTENDACTIONFACTORY_H
