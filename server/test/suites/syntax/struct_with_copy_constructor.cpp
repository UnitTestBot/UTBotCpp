/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "struct_with_copy_constructor.h"

Closet::Closet(const Closet& other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
    flag = true;
}

Closet::Closet() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
    flag = false;
}

Closet::Closet(double length_, double width_, double height_) {
    length = length_;
    width = width_;
    height = height_;
    volume = height * width * length;
}

Closet::Closet(Closet && other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
}