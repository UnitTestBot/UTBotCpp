/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_INNER_UNNAMED_H
#define UNITTESTBOT_INNER_UNNAMED_H

struct StructWithUnnamedUnion {
    union {
        char c;
        int x;
    };

    int y;
};

union UnionWithUnnamedStruct {
    struct {
        char c;
        int x;
    };

    int y;
};

struct StructWithUnnamedUnion struct_with_unnamed_union_as_return_type(char t);

struct StructWithUnnamedUnion struct_with_unnamed_union_as_parameter_type(struct StructWithUnnamedUnion swuu);

union UnionWithUnnamedStruct union_with_unnamed_struct_as_return_type(char t);

union UnionWithUnnamedStruct union_with_unnamed_struct_as_parameter_type(union UnionWithUnnamedStruct uwus);

#endif //UNITTESTBOT_INNER_UNNAMED_H
