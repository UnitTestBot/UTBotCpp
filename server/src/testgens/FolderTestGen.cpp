#include "FolderTestGen.h"

#include "Paths.h"

FolderTestGen::FolderTestGen(const testsgen::FolderRequest &request,
                             ProgressWriter *progressWriter,
                             bool testMode)
    : ProjectTestGen(request.projectrequest(), progressWriter, testMode, false),
      folderPath(request.folderpath()) {
    testingMethodsSourcePaths = CollectionUtils::filterOut(sourcePaths,
                 [this](const fs::path &path) { return !Paths::isSubPathOf(folderPath, path); });
    setInitializedTestsMap();
}

std::string FolderTestGen::toString() {
    std::stringstream s;
    s << ProjectTestGen::toString() << "folder path: " << folderPath << "\nfile paths:\n";
    for (const auto &sp : testingMethodsSourcePaths) {
        s << sp.string() << "; ";
    }
    s << "\n";
    return s.str();
}
