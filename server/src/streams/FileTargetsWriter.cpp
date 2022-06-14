#include "FileTargetsWriter.h"

void FileTargetsWriter::writeResponse(
    const std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> &targets,
    const utbot::ProjectContext &projectContext) {
    if (!hasStream()) {
        return;
    }
    writeTargets(targets, projectContext);
}
