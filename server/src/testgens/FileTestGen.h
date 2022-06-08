#ifndef UNITTESTBOT_FileTESTGEN_H
#define UNITTESTBOT_FileTESTGEN_H

#include "ProjectTestGen.h"


class FileTestGen final : public ProjectTestGen {
public:
    fs::path filepath;

    FileTestGen(const testsgen::FileRequest &request,
                ProgressWriter *progressWriter,
                bool testMode);

    ~FileTestGen() override = default;

    std::string toString() override;
};


#endif // UNITTESTBOT_FileTESTGEN_H
