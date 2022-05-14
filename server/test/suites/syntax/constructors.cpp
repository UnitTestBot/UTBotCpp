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

BufferOfFiveElements::BufferOfFiveElements(int vec_[], int size) {
    first = vec_[0];
    last = vec_[size - 1];
    for (int i = 0; i < size; i++) {
        vec[i] = vec_[i];
    }
    assert(last == vec[4]);
    assert(size == 5);
}

BufferOfFiveElements::BufferOfFiveElements() {
    first = 0;
    last = 0;
    vec[0] = vec[1] = vec[2] = vec[3] = vec[4] = 0;
}

Closet::Closet(const Closet& other) {
    length = other.length;
    width = other.width;
    height = other.height;
    volume = other.volume;
    assert(volume == length * width * height);
}

Closet::Closet() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
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
    assert(volume == length * width * height);
}

Closet3::Closet3() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
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
