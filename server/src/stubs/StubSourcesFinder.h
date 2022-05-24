/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_STUBSOURCESFINDER_H
#define UNITTESTBOT_STUBSOURCESFINDER_H

#include "building/BuildDatabase.h"
#include "stubs/Stubs.h"

#include "utils/path/FileSystemPath.h"
#include <unordered_set>
#include <vector>

class StubSourcesFinder {
public:
    explicit StubSourcesFinder(std::shared_ptr<BuildDatabase> buildDatabase);

    std::vector<fs::path> find(const fs::path& testedFilePath);

    std::vector<fs::path> excludeFind(const fs::path &testedFilePath, const fs::path &rootPath);

    void printAllModules();

private:
    std::shared_ptr<BuildDatabase> buildDatabase;

    CollectionUtils::FileSet getLibraryBitcodeFiles(const fs::path &testedFilePath);
};


#endif // UNITTESTBOT_STUBSOURCESFINDER_H
