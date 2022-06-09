/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SETTINGSCONTEXT_H
#define UNITTESTBOT_SETTINGSCONTEXT_H

#include <chrono>
#include <optional>
#include <protobuf/testgen.grpc.pb.h>
#include <utils/ErrorMode.h>

namespace testsgen {
    class SettingsContext;
}

namespace utbot {
    class SettingsContext {
    public:
        explicit SettingsContext(const testsgen::SettingsContext &settingsContext);

        SettingsContext(bool generateForStaticFunctions,
                        bool verbose,
                        int32_t timeoutPerFunction,
                        int32_t timeoutPerTest,
                        bool useDeterministicSearcher,
                        bool useStubs,
                        ::testsgen::ErrorMode errorMode);

        const bool generateForStaticFunctions;
        const bool verbose;
        const std::optional<std::chrono::seconds> timeoutPerFunction, timeoutPerTest;
        const bool useDeterministicSearcher;
        const bool useStubs;
        ::testsgen::ErrorMode errorMode;
    };
}


#endif // UNITTESTBOT_SETTINGSCONTEXT_H
