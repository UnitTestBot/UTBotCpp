#ifndef UNITTESTBOT_FolderTESTGEN_H
#define UNITTESTBOT_FolderTESTGEN_H

#include "ProjectTestGen.h"


class FolderTestGen final : public ProjectTestGen {
public:
    std::string folderPath;

    FolderTestGen(const testsgen::FolderRequest &request,
                  ProgressWriter *progressWriter,
                  bool testMode);

    ~FolderTestGen() override = default;

    std::string toString() override;
};


#endif // UNITTESTBOT_FolderTESTGEN_H
