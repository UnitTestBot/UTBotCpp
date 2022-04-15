/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
*/

#include "struct_with_default_contsructor.h"

Closet2::Closet2() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
}

Closet2::Closet2(double length_, double width_, double height_) {
    length = length_;
    width = width_;
    height = height_;
    volume = height * width * length;
}

Closet2::Closet2(const Closet2& other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
}

Closet2::Closet2(Closet2 && other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
}