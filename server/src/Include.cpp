/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Include.h"

#include <utility>

Include::Include(bool isAngled, fs::path path) : is_angled(isAngled), path(std::move(path)) {}
