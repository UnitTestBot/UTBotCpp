#include "structs_with_pointers.h"
#include <stddef.h>

int handle_struct_with_char_ptr(struct StructWithCharPointer st) {
    if (st.a == 0 && st.str != NULL) {
        return 0;
    } else if (st.a > 0) {
        return 1;
    } else {
        return 2;
    }
}

int list_sum(struct List *head) {
    struct List *cur = head;
    int res = 0;
    while (cur != NULL) {
        res += cur->val;
        cur = cur->next;
    }
    return res;
}

int list_sum_sign(struct List *head) {
    int sum = list_sum(head);
    if (sum > 0) {
        return 1;
    } else if (sum < 0) {
        return -1;
    } else {
        return 0;
    }
}

int unsafe_next_value(struct List *node) {
    return node->next->val;
}

long long sum_of_all_fields_or_mult(struct StructWithPointerInField st, int a) {
    if (a == -322 && st.ll * st.sh * st.a.a != 0) {
        return st.ll + st.sh + st.a.a;
    } else if (st.ll * st.sh * st.a.a != 0) {
        return 11;
    } else if (st.ll > 0) {
        return st.ll;
    }
    return 1132;
}

struct StructWithPointerInField some_calc(int a, int b) {
    struct StructWithPointerInField st;
    if (a == 0 && b == 0) {
        st.ll = 11111;
        st.sh = 11;
        st.a.a = 1;
        st.a.str = "AAA";
        return st;
    } else if (a > 0 && b > 0) {
        st.ll = 22222;
        st.sh = 22;
        st.a.a = 2;
        st.a.str = "BBB";
    } else if (a * b < 0) {
        st.ll = 33333;
        st.sh = 33;
        st.a.a = 3;
        st.a.str = "CCC";
        return st;
    }
    st.ll = 0;
    st.sh = 0;
    st.a.a = 0;
    st.a.str = "DDD";
    return st;
}

int sumStructWithPointer(struct StructWithPointer par) {
    return par.x + *par.y;
}

int sumStructWithPointerAsPointer(struct StructWithPointer* par) {
    return par->x + *par->y;
}

int sumStructWithDoublePointer(struct StructWithDoublePointer par) {
    return par.x + **par.y;
}

int sumStructWithArrayOfPointer(struct StructWithArrayOfPointer par) {
    return par.x + *(par.y[0]) + *(par.y[1]);
}

int sumStructWithStructWithPointer(struct StructWithStructWithPointer par) {
    int sswp = sumStructWithPointer(par.swp);
    int sswdp = sumStructWithDoublePointer(*par.swdp);
    return sswp + sswdp;
}

int sumStructManyPointers(struct StructManyPointers par) {
    return par.a + *par.b + **par.c + ***par.d;
}

int sumStructComplex(struct StructComplex par) {
    int sswp = sumStructWithPointer(*par.swp);
    int sswdp = sumStructWithDoublePointer(**par.swdp);
    int ssmp = sumStructManyPointers(par.smp);
    return par.x + *par.y + **par.z + sswp + sswdp + ssmp;
}

int sumStructWithOnePointer(struct StructWithPointer *par) {
    return par->x + *par->y;
}

int sumConstStructWithOnePointer(const struct StructWithPointer *par) {
    return par->x + *par->y;
}
