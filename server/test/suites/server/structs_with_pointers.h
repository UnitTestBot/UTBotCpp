#ifndef UNITTESTBOT_STRUCTS_WITH_POINTERS_H
#define UNITTESTBOT_STRUCTS_WITH_POINTERS_H

typedef int (*some_func)();

struct InnerStruct {
    some_func func;
    int func_id;
};

struct MainStruct {
    struct InnerStruct* inner;
};

int process_struct_with_func_pointer(struct MainStruct* str);

#endif // UNITTESTBOT_STRUCTS_WITH_POINTERS_H
