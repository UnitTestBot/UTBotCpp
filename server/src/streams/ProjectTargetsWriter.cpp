#include "ProjectTargetsWriter.h"

ProjectTargetsWriter::ProjectTargetsWriter(testsgen::ProjectTargetsResponse *response)
    : TargetsWriter(response) {
}

void ProjectTargetsWriter::writeResponse(
    const utbot::ProjectContext &projectContext,
    const std::vector<fs::path> &targetPaths) {
    if (!hasStream()) {
        return;
    }
    writeTargets(targetPaths, projectContext);
    auto utbotAutoTarget = std::make_unique<testsgen::ProjectTarget>(GrpcUtils::createAutoTarget());
    writer->set_allocated_prioritytarget(utbotAutoTarget.release());
}
