#ifndef UNITTESTBOT_FILETARGETSWRITER_H
#define UNITTESTBOT_FILETARGETSWRITER_H

#include "TargetsWriter.h"

#include <protobuf/testgen.pb.h>

class FileTargetsWriter : public TargetsWriter<testsgen::FileTargetsResponse> {
public:
    explicit FileTargetsWriter(testsgen::FileTargetsResponse *response)
        : TargetsWriter(response) {
    }

    void writeResponse(const std::vector<fs::path> &targetPaths,
                       const utbot::ProjectContext& projectContext);
};


#endif // UNITTESTBOT_FILETARGETSWRITER_H
