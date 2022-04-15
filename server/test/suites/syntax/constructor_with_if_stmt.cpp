/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
*/
#include "constructor_with_if_stmt.h"

constructor_with_if_stmt::constructor_with_if_stmt(bool isBig) {
    if (isBig) {
        number = 10000;
    } else {
        number = 1;
    }
}
constructor_with_if_stmt::constructor_with_if_stmt() {
    number = 0;
}
