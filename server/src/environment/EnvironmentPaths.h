/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_ENVIRONMENT_PATHS_H
#define UNITTESTBOT_ENVIRONMENT_PATHS_H

#include "utils/path/FileSystemPath.h"

namespace Paths {
    fs::path getUTBotRootDir();
    
    fs::path getUTBotInstallDir();

    fs::path getUTBotDebsInstallDir();

    fs::path getPython();

    fs::path getBear();

    fs::path getCMake();

    fs::path getMake();

    fs::path getUTBotClang();
    
    fs::path getUTBotClangPP();

    fs::path getLLVMnm();

    fs::path getGtestLibPath();

    fs::path getLLVMprofdata();
    fs::path getLLVMcov();

    fs::path getLLVMgold();

    fs::path getAr();

    fs::path getLdGold();

    fs::path getLd();

  // Gcc is used only in tests
    fs::path getGcc();
    fs::path getGpp();
}

#endif //UNITTESTBOT_ENVIRONMENT_PATHS_H