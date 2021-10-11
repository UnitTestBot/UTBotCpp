/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_THREADSAFECONTAINERS_H
#define UNITTESTBOT_THREADSAFECONTAINERS_H

#include "Paths.h"
#include "utils/JsonUtils.h"

#include <nlohmann/json.hpp>

#include "utils/path/FileSystemPath.h"
#include <fstream>
#include <mutex>
#include <unordered_set>

template<class Value>
class concurrent_set {
private:
    std::unordered_set<Value> set;
    mutable std::mutex mutex;
public:
    void insert(const Value &value) {
        std::lock_guard<std::mutex> guard(mutex);
        set.insert(value);
    }

    bool in(const Value &value) const {
        std::lock_guard<std::mutex> guard(mutex);
        return (set.find(value) != set.end());
    }

    void writeToJson() const {
        std::lock_guard<std::mutex> guard(mutex);
        nlohmann::json j(set);
        fs::path jsonPath = Paths::getClientsJsonPath();
        JsonUtils::writeJsonToFile(jsonPath, j);
    }
};

#endif //UNITTESTBOT_THREADSAFECONTAINERS_H
