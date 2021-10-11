/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_UNITTEST_H
#define UNITTESTBOT_UNITTEST_H

#include "utils/path/FileSystemPath.h"
#include <string>

struct UnitTest {
    fs::path testFilePath;
    std::string suitename;
    std::string testname;
};

#endif //UNITTESTBOT_UNITTEST_H
