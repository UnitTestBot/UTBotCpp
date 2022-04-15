/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "struct_with_parametrized_constructor.h"

Closet4::Closet4(double length_, double width_, double height_) {
    length = length_;
    width = width_;
    height = height_;
    volume = height * width * length;
}

Closet4::Closet4() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
}

Closet4::Closet4(const Closet4& other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
}

Closet4::Closet4(Closet4 && other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
}