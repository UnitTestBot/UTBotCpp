#ifndef UNITTESTBOT_STUBSOURCESFINDER_H
#define UNITTESTBOT_STUBSOURCESFINDER_H

#include "building/ProjectBuildDatabase.h"
#include "stubs/Stubs.h"

#include "utils/path/FileSystemPath.h"
#include <unordered_set>
#include <vector>

class StubSourcesFinder {
public:
    explicit StubSourcesFinder(std::shared_ptr<ProjectBuildDatabase> buildDatabase);

    std::vector<fs::path> find(const fs::path& testedFilePath);

    std::vector<fs::path> excludeFind(const fs::path &testedFilePath, const fs::path &rootPath);

    void printAllModules();

private:
    std::shared_ptr<ProjectBuildDatabase> buildDatabase;

    CollectionUtils::FileSet getLibraryBitcodeFiles(const fs::path &testedFilePath);
};


#endif // UNITTESTBOT_STUBSOURCESFINDER_H
