#ifndef UNITTESTBOT_ClassTESTGEN_H
#define UNITTESTBOT_ClassTESTGEN_H

#include "LineTestGen.h"
#include "ProjectTestGen.h"

class ClassTestGen final : public LineTestGen {
public:
    ClassTestGen(const testsgen::ClassRequest &request,
                 ProgressWriter *progressWriter,
                 bool testMode);

    ~ClassTestGen() override = default;

    std::string toString() override;

    bool needToAddPathFlag() override;
};


#endif // UNITTESTBOT_ClassTESTGEN_H
