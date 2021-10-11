/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

class Refs {
public:
    Refs() = default;

    int foo(int& a);

    int& bar(char c);

private:
    int intRef;
};