/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
*/

#ifndef UNITTESTBOT_STRUCTS_WITH_DEFAULT_CONSTRUCTORS_H
#define UNITTESTBOT_STRUCTS_WITH_DEFAULT_CONSTRUCTOR_H
#include <cassert>

struct Closet2 {
    double length;
    double width;
    double height;
    double volume;
    
    Closet2();
    Closet2(double length_, double width_, double height_);
    Closet2(const Closet2& other);
    Closet2(Closet2 && other);
};

#endif // UNITTESTBOT_STRUCTS_WITH_CONSTRUCTORS_H
