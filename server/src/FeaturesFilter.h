/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FEATURESFILTER_H
#define UNITTESTBOT_FEATURESFILTER_H

#include "Paths.h"
#include "SettingsContext.h"
#include "Tests.h"
#include "types/Types.h"
#include "utils/ExecUtils.h"

#include <type_traits>
#include <vector>

class FeaturesFilter {
public:
    static void filter(utbot::SettingsContext const &settingsContext,
                       const types::TypesHandler &typesHandler,
                       tests::TestsMap &testsMap,
                       bool throwIfZeroFunctions = true);

private:
    static void filterGlobalParameters(const types::TypesHandler &typesHandler,
                                       tests::Tests &tests);
};


#endif // UNITTESTBOT_FEATURESFILTER_H
