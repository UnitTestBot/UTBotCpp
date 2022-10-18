#include "FileTargetsWriter.h"

void FileTargetsWriter::writeResponse(
    const std::vector<fs::path> &targetPaths,
    const utbot::ProjectContext &projectContext) {
    if (!hasStream()) {
        return;
    }
    writeTargets(targetPaths, projectContext);
}
