#ifndef UNITTESTBOT_RETURNTYPESFETCHER_H
#define UNITTESTBOT_RETURNTYPESFETCHER_H


#include "streams/ProgressWriter.h"
#include "utils/CollectionUtils.h"

#include "utils/path/FileSystemPath.h"
#include <vector>

class BaseTestGen;

class ReturnTypesFetcher {
private:
    BaseTestGen *testGen;

public:
    explicit ReturnTypesFetcher(BaseTestGen *testGen) : testGen(testGen) {
    }

    void fetch(ProgressWriter *const progressWriter, const CollectionUtils::FileSet &allFiles);
};


#endif // UNITTESTBOT_RETURNTYPESFETCHER_H
