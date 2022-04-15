/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "struct_with_move_constructor.h"

Closet3::Closet3(Closet3 && other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
    flag = true;
}

Closet3::Closet3() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
    flag = false;
}

Closet3::Closet3(double length_, double width_, double height_) {
    length = length_;
    width = width_;
    height = height_;
    volume = height * width * length;
}

Closet3::Closet3(const Closet3& other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
}
