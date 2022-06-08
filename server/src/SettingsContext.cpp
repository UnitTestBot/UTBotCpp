/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "SettingsContext.h"

#include <protobuf/testgen.grpc.pb.h>

namespace utbot {
    SettingsContext::SettingsContext(bool generateForStaticFunctions,
                                     bool verbose,
                                     int32_t timeoutPerFunction,
                                     int32_t timeoutPerTest,
                                     bool useDeterministicSearcher,
                                     bool useStubs,
                                     ::testsgen::SettingsContext_ErrorMode errorMode)
        : generateForStaticFunctions(generateForStaticFunctions),
          verbose(verbose),
          timeoutPerFunction(timeoutPerFunction > 0
                                 ? std::make_optional(std::chrono::seconds{ timeoutPerFunction })
                                 : std::nullopt),
         timeoutPerTest(timeoutPerTest > 0
         ? std::make_optional(std::chrono::seconds{ timeoutPerTest })
         : std::nullopt),
          useDeterministicSearcher(useDeterministicSearcher), useStubs(useStubs),
          errorMode(static_cast<ErrorMode::ErrorMode>(errorMode)) {
    }
    SettingsContext::SettingsContext(const testsgen::SettingsContext &settingsContext)
        : SettingsContext(settingsContext.generateforstaticfunctions(),
                          settingsContext.verbose(),
                          settingsContext.timeoutperfunction(),
                          settingsContext.timeoutpertest(),
                          settingsContext.usedeterministicsearcher(),
                          settingsContext.usestubs(),
                          settingsContext.errormode()) {
    }
}