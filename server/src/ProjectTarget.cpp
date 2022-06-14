#include "ProjectTarget.h"

#include <protobuf/testgen.grpc.pb.h>

#include <utility>

namespace utbot {
    ProjectTarget::ProjectTarget(std::string name, fs::path path)
        : name(std::move(name)), path(std::move(path)) {
    }
    ProjectTarget::ProjectTarget(const testsgen::ProjectTarget &projectTarget)
        : ProjectTarget(projectTarget.name(), projectTarget.path()) {
    }

    const std::string &ProjectTarget::getName() const {
        return name;
    }
    const fs::path &ProjectTarget::getPath() const {
        return path;
    }
}
