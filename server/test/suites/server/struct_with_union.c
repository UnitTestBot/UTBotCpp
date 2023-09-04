#include "struct_with_union.h"

struct StructWithUnionOfUnnamedType struct_with_union_of_unnamed_type_as_return_type(int a, int b) {
    struct StructWithUnionOfUnnamedType ans;
    if (a > b) {
        ans.un.ptr = 0;
    } else if (a < b) {
        ans.un.x = 153;
    } else {
        ans.un.ds.c = 'k';
        ans.un.ds.d = 1.0101;
    }
    return ans;
}

struct StructWithAnonymousUnion struct_with_anonymous_union_as_return_type(int a, int b) {
    struct StructWithAnonymousUnion ans;
    if (a > b) {
        ans.ptr = 0;
    } else if (a < b) {
        ans.x = 153;
    } else {
        ans.c = 'k';
        ans.d = 1.0101;
    }
    return ans;
}