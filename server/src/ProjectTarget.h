/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PROJECTTARGET_H
#define UNITTESTBOT_PROJECTTARGET_H

#include "utils/path/FileSystemPath.h"

#include <string>

namespace testsgen {
    class ProjectTarget;
}

namespace utbot {
    class ProjectTarget {
    public:
        ProjectTarget(std::string name, fs::path path);

        explicit ProjectTarget(testsgen::ProjectTarget const &projectTarget);

        const std::string &getName() const;
        const fs::path &getPath() const;

    private:
        std::string name;
        fs::path path;
    };
}


#endif // UNITTESTBOT_PROJECTTARGET_H
