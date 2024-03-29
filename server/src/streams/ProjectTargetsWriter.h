#ifndef UNITTESTBOT_PROJECTTARGETSWRITER_H
#define UNITTESTBOT_PROJECTTARGETSWRITER_H

#include "TargetsWriter.h"
#include "building/BuildDatabase.h"

#include <protobuf/testgen.pb.h>

#include <vector>

class ProjectTargetsWriter : public TargetsWriter<testsgen::ProjectTargetsResponse> {
public:
    explicit ProjectTargetsWriter(testsgen::ProjectTargetsResponse *response);

    void writeResponse(const utbot::ProjectContext &projectContext,
                       const std::vector<fs::path> &targetPaths);
};


#endif // UNITTESTBOT_PROJECTTARGETSWRITER_H
