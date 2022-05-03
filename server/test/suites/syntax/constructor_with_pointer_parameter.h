/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_CONSTRUCTOR_WITH_POINTER_PARAMETER_H
#define UNITTESTBOT_CONSTRUCTOR_WITH_POINTER_PARAMETER_H


struct constructor_with_pointer_parameter {
    int first;
    int last;
    int vec[5];
    constructor_with_pointer_parameter();
    constructor_with_pointer_parameter(int vec[], int size);
};


#endif // UNITTESTBOT_CONSTRUCTOR_WITH_POINTER_PARAMETER_H
