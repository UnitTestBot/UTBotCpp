#include "simple_structs.h"

int get_sign_struct(struct MyStruct st) {
    if (st.a == 0) {
        return 0;
    }
    if (st.a < 0) {
        return -1;
    } else {
        return 1;
    }
}

int calculate_something(struct OneMoreStruct str) {
    if (str.a == 1 && str.b == 10754) {
        return 0;
    }
    if (str.c != 0 && str.a == 7 && str.b == 2020) {
        return 1;
    }
    if (str.c != 0) {
        return (int)str.c;
    }
    if ((long long) str.a > str.b) {
        return str.a;
    } else {
        return (int)str.b;
    }
}

char get_symbol_by_struct(const struct StructWithChars st) {
    if (st.a == 'a') {
        return 'a';
    }
    if (st.c == 'c') {
        return 'c';
    }

    if (st.u == 'u') {
        return 'u';
    }

    if (st.b == 1) {
        return '1';
    }

    return '0';
}

signed char operate_with_inner_structs(struct MainStruct st) {
    if (st.x == 5 && st.y == 5 && st.inner.c == '5' && 
        st.inner.s == 5 && st.inner.ininner.l == 5 && st.inner.ininner.u == 5) {
            return '5';
    }

    if (st.x == 5 && st.y == 101 && st.inner.s == 15) {
        return st.inner.c;
    }

    if ((long long) st.inner.ininner.u == st.inner.ininner.l) {
        return 'e';
    }
    if ((long long) st.inner.ininner.u > st.inner.ininner.l) {
        return 'g';
    }

    return 'o';
}


struct MainStruct struct_as_return_type(int a) {
    if (a == 0) {
        struct MainStruct res = {{'0', {0, 0}, 0}, 0, 0}; 
        return res;
    }

    if (a == 1) {
        struct MainStruct res = {{'1', {1, 1}, 1}, 1, 1}; 
        return res;
    }

    struct MainStruct res = {{'2', {2, 2}, 2}, 2, 2};
    return res;
}

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

struct A complex_struct_with_union_as_return_type(int t)
{
    struct A ans;
    if (t == 5) {
        ans.a.t.arr.c = 'u';
    } else {
        ans.a.t.arr.y = 3.14;
    }
    return ans;
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
