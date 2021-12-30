/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_MULTIPLE_CLASSES_H
#define UNITTESTBOT_MULTIPLE_CLASSES_H


class first_class {
public:
    int get1();
};

struct second_class {
public:
    struct third_class {
    public:
        int get3();
    };
    int get2();
};


#endif //UNITTESTBOT_MULTIPLE_CLASSES_H
