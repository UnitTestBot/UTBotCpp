/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "constructors.h"
#include <cassert>

BigOrSmallInteger::BigOrSmallInteger(bool isBig) {
    if (isBig) {
        number = 10000;
    } else {
        number = 1;
    }
}

BigOrSmallInteger::BigOrSmallInteger() {
    number = 0;
}

TwoElements::TwoElements(int *first_, int *second_) {
    if (*first_ > *second_) {
        first = *first_;
        second = *second_;
    } else {
        first = *second_;
        second = *first_;
    }
}

TwoElements::TwoElements() {
    first = 0;
    second = 0;
}

Closet::Closet(const Closet& other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
}

Closet::Closet() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
}

Closet::Closet(double length_, double width_, double height_, double volume_) {
    length = length_;
    width = width_;
    height = height_;
    volume = volume_;
}

Closet2::Closet2() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
}

Closet3::Closet3(Closet3 && other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
}

Closet3::Closet3() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
}

Closet3::Closet3(double length_, double width_, double height_, double volume_) {
    length = length_;
    width = width_;
    height = height_;
    volume = volume_;
}

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
