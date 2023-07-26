#include "structs_with_pointers.h"

int process_struct_with_func_pointer(struct MainStruct* str) {
    if (str && str->inner && str->inner->func_id != 0) {
        return 1;
    }
    return 0;
}