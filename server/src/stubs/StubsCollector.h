/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_STUBSCOLLECTOR_H
#define UNITTESTBOT_STUBSCOLLECTOR_H

#include "Tests.h"

class StubsCollector {
    const types::TypesHandler *const typesHandler;

public:
    explicit StubsCollector(const types::TypesHandler &typesHandler);

    void collect(tests::TestsMap &testsMap);
};


#endif // UNITTESTBOT_STUBSCOLLECTOR_H
