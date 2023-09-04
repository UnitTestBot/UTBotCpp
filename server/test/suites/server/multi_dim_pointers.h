#ifndef UNITTESTBOT_MULTI_DIM_POINTERS_H
#define UNITTESTBOT_MULTI_DIM_POINTERS_H

struct ElementStruct {
    struct ElementStruct *prev;
    struct ElementStruct *next;
};

struct ListStruct {
    struct ElementStruct *head;
    struct ElementStruct *tail;
    unsigned size;
};

struct MainStruct {
    struct ListStruct list;
    const char **name;
};

int func_with_multi_dim_pointer(struct MainStruct **str);

int func_with_multi_dim_pointer_to_const(const struct MainStruct **str);

#endif // UNITTESTBOT_MULTI_DIM_POINTERS_H
