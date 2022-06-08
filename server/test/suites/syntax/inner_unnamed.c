#include "inner_unnamed.h"

struct StructWithUnnamedUnion struct_with_unnamed_union_as_return_type(char t) {
    struct StructWithUnnamedUnion swuu;
    swuu.x = t;
    swuu.y = t;
    return swuu;
}

struct StructWithUnnamedUnion struct_with_unnamed_union_as_parameter_type(struct StructWithUnnamedUnion swuu) {
    if (swuu.x == 0 && swuu.y == 0) {
        swuu.x = 42;
        swuu.y = 42;
        return swuu;
    }
    swuu.x = 24;
    swuu.y = 24;
    return swuu;
}

union UnionWithUnnamedStruct union_with_unnamed_struct_as_return_type(char t) {
    union UnionWithUnnamedStruct uwus;
    uwus.y = 0;
    uwus.c = t;
    uwus.x = t;
    return uwus;
}

union UnionWithUnnamedStruct union_with_unnamed_struct_as_parameter_type(union UnionWithUnnamedStruct uwus) {
    if (uwus.c == 0 && uwus.x == 0 && uwus.y == 0) {
        uwus.y = 42;
        uwus.c = 42;
        uwus.x = 42;
        return uwus;
    }
    uwus.y = 24;
    uwus.c = 24;
    uwus.x = 24;
    return uwus;
}
