/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_STRUCT_WITH_COPY_CONTSRUCTOR_H
#define UNITTESTBOT_STRUCT_WITH_COPY_CONTSRUCTOR_H

#include <cassert>

struct Closet {
    double length;
    double width;
    double height;
    double volume;
    bool flag = false;

    Closet();
    Closet(double length_, double width_, double height_);
    Closet(const Closet &other);
    Closet(Closet &&other);
};

#endif // UNITTESTBOT_STRUCT_WITH_COPY_CONTSRUCTOR_H
