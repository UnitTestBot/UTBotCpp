#include "multi_dim_pointers.h"

int func_with_multi_dim_pointer(struct MainStruct **str) {
    if (!str) {
        return 0;
    }
    str++;
    struct MainStruct *ptr = *str;
    int sz = 0;
    if (ptr) {
        if (!ptr->name) {
            return -1;
        }
        struct ElementStruct *e = ptr->list.head;
        struct ElementStruct *n;
        for (int i = 0; i < 5; i++) {
            if (e) {
                n = e->next;
                sz++;
            } else {
                break;
            }
        }
    }
    return sz;
}

int func_with_multi_dim_pointer_to_const(const struct MainStruct **str) {
    if (!str) {
        return 0;
    }
    str++;
    struct MainStruct *ptr = *str;
    int sz = 0;
    if (ptr) {
        struct ElementStruct *e = ptr->list.head;
        struct ElementStruct *n;
        for (int i = 0; i < 5; i++) {
            if (e) {
                n = e->next;
                sz++;
            } else {
                break;
            }
        }
    }
    return sz;
}
