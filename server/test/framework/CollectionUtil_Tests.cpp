/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "gtest/gtest.h"

#include "utils/CollectionUtils.h"

#include "utils/path/FileSystemPath.h"
#include <vector>

using namespace std::placeholders;

namespace {
    using std::bind;
    using std::string;
    using std::vector;

    auto projectPath = fs::current_path().parent_path() / "test/suites/server";

    TEST(CollectionUtil_Test, erase) {
        const fs::path &bitcodeFile = projectPath / "out.bc";
        std::vector<fs::path> files = { projectPath / "out_klee.bc",
                                        bitcodeFile,
                                        bitcodeFile };
        EXPECT_FALSE(CollectionUtils::erase(files, projectPath));
        EXPECT_EQ(3, files.size());
        EXPECT_TRUE(CollectionUtils::erase(files, bitcodeFile));
        EXPECT_EQ(1, files.size());
        EXPECT_FALSE(CollectionUtils::erase(files, bitcodeFile));
        EXPECT_EQ(1, files.size());
    }

    TEST(CollectionUtil_Test, erase_if) {
        std::vector<fs::path> files = { projectPath / "out.bc",
                                        projectPath / "dir",
                                        projectPath / "out.c.bc" };
        auto erased = CollectionUtils::erase_if(files, std::bind(&fs::path::has_extension, _1));
        EXPECT_EQ(1, files.size());
        EXPECT_EQ(2, erased);
    }

    TEST(CollectionUtil_Test, transform) {
        vector<vector<int>> items = { { 1, 2, 3 }, { 4, 5 }, { 6, 7 } };
        vector<size_t> sizes = CollectionUtils::transform(items, bind(&vector<int>::size, _1));
        EXPECT_EQ((vector<size_t>{ 3, 2, 2 }), sizes);
    }

    TEST(CollectionUtil_Test, transformTo) {
        vector<vector<int>> items = { { 1, 2, 3 }, { 4, 5 }, { 6, 7 } };
        std::vector<int> sizes = CollectionUtils::transformTo<std::vector<int>>(items, bind(&vector<int>::size, _1));
        EXPECT_EQ((std::vector<int>{ 3, 2, 2 }), sizes);
    }

    TEST(CollectionUtil_Test, extend) {
        vector<int> items1 = { 1, 2, 3 };
        vector<int> items2 = { 4, 5 };
        CollectionUtils::extend(items1, items2);
        EXPECT_EQ((vector<int>{ 1, 2, 3, 4, 5 }), items1);
        EXPECT_EQ((vector<int>{ 4, 5 }), items2);
    }

    TEST(CollectionUtil_Test, contains) {
        vector<int> items = { 1, 2, 3, 1 };
        EXPECT_TRUE(CollectionUtils::contains(items, 1));
        EXPECT_FALSE(CollectionUtils::contains(items, -1));
    }
}