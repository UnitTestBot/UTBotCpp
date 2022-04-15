/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
*/
#include "constructor_with_pointer_parameter.h"
#include <cassert>
constructor_with_pointer_parameter::constructor_with_pointer_parameter(int vec_[], int size) {
    first = vec_[0];
    last = vec_[size - 1];
    for (int i = 0; i < size; i++) {
        vec[i] = vec_[i];
    }
    assert(last == vec[4]);
    assert(size == 5);
}

constructor_with_pointer_parameter::constructor_with_pointer_parameter() {
    first = 0;
    last = 0;
    vec[0] = vec[1] = vec[2] = vec[3] = vec[4] = 0;
}
