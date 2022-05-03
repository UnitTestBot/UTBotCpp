/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_STRUCT_WITH_MOVE_CONTSRUCTOR_H
#define UNITTESTBOT_STRUCT_WITH_MOVE_CONTSRUCTOR_H

#include <cassert>

struct Closet3 {
    double length;
    double width;
    double height;
    double volume;
    bool flag;

    Closet3();
    Closet3(double length_, double width_, double height_);
    Closet3(const Closet3& other);
    Closet3(Closet3 && other);
};

#endif // UNITTESTBOT_STRUCT_WITH_MOVE_CONTSRUCTOR_H
