/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

class SimpleClass {
public:
    SimpleClass();

    int sum_with_c(int a);

    int sub_with_c(int a, int b);

    int mul_with_c(int a, int b);
private: 
    int c;
};

int outer_func(int a, int b);