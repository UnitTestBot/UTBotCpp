#ifndef UNITTESTBOT_FunctionTESTGEN_H
#define UNITTESTBOT_FunctionTESTGEN_H

#include "LineTestGen.h"
#include "ProjectTestGen.h"

class FunctionTestGen final : public LineTestGen {
public:
    FunctionTestGen(const testsgen::FunctionRequest &request,
                    ProgressWriter *progressWriter,
                    bool testMode);

    ~FunctionTestGen() override = default;

    std::string toString() override;

    bool needToAddPathFlag() override;
};


#endif // UNITTESTBOT_FunctionTESTGEN_H
