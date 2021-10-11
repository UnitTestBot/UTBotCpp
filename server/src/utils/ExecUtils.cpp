/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ExecUtils.h"

namespace ExecUtils {
    void throwIfCancelled() {
        auto context = RequestEnvironment::getServerContext();
        if (context && context->IsCancelled()) {
            throw CancellationException();
        }
    }

    void toCArgumentsPtr(vector<std::string> &argv,
                         vector<std::string> &envp,
                         vector<char *> &cargv,
                         vector<char *> &cenvp,
                         bool appendNull) {
        for (auto &s : argv) {
            cargv.emplace_back((char*)s.data());
        }
        for (auto &s : envp) {
            cenvp.emplace_back((char*)s.data());
        }
        if (appendNull) {
            cargv.emplace_back(nullptr);
        }
        cenvp.emplace_back(nullptr);
    }

    vector<std::string> environAsVector() {
        static vector <std::string> res;
        if (res.empty()) {
            char **env = environ;
            for (int i = 0; env[i]; i++) {
                res.emplace_back(env[i]);
            }
        }
        return res;
    }
}
