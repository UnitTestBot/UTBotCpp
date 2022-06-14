#ifndef UNITTESTBOT_PredicateTESTGEN_H
#define UNITTESTBOT_PredicateTESTGEN_H

#include "LineTestGen.h"
#include "ProjectTestGen.h"

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>

class PredicateTestGen final : public LineTestGen {
public:
    std::string predicate;
    std::string returnValue;
    testsgen::ValidationType type;

    explicit PredicateTestGen(const testsgen::PredicateRequest &request,
                              ProgressWriter *progressWriter,
                              bool testMode);

    ~PredicateTestGen() override = default;

    std::string toString() override;

    bool needToAddPathFlag() override;
};


#endif // UNITTESTBOT_PredicateTESTGEN_H
