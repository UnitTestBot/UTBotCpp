/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

struct String {
    char c[16];
};

struct String** global;

char first() {
   return global[0][0].c[0];
}