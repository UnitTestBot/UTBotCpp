#ifndef UNITTESTBOT_TARGETSWRITER_H
#define UNITTESTBOT_TARGETSWRITER_H

#include "MessageWriter.h"
#include "building/BuildDatabase.h"

template <typename Response>
class TargetsWriter : public MessageWriter<Response> {
public:
    explicit TargetsWriter(Response *response) : MessageWriter<Response>(response) {
    }

protected:
    using MessageWriter<Response>::writer;

    void writeTargets(const std::vector<fs::path> &targetPaths,
                      const utbot::ProjectContext &projectContext) {
        auto projectTarget = writer->add_targets();
        *projectTarget = GrpcUtils::createAutoTarget();
        for (auto const &target : targetPaths) {
            projectTarget = writer->add_targets();
            GrpcUtils::initProjectTarget(*projectTarget, projectContext, target);
        }
    }
};


#endif // UNITTESTBOT_TARGETSWRITER_H
