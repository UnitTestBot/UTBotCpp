#include "struct_with_union.h"

struct StructWithUnion struct_with_union_as_return_type(int t) {
    struct StructWithUnion st;
    if (t == 0) {
        st.un.c = 'a';
        st.is.un2.x = 101;
        st.x = 155;
    } else {
        st.un.x = 17;
        st.is.un2.c = '0';
        st.x = -108;
    }
    return st;
}

struct StructWithUnionInUnion struct_with_union_in_union_as_return_type(int a, int b) {
    struct StructWithUnionInUnion ans;
    if (a + b > 16) {
        ans.un.x = -5;
    } else if (a + b < 0) {
        ans.un.c = 'b';
    } else {
        ans.un.t.y = 1.41;
    }
    return ans;
}

struct StructWithStructInUnion struct_with_struct_in_union_as_return_type(int a, int b) {
    struct StructWithStructInUnion ans;
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
