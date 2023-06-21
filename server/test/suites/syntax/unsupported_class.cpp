#include "unsupported_class.h"

int private_array::sum() {
    return values[0] + values[1];
}

private_array return_private_array() {
    return private_array();
}

bool parameter_private_array(private_array pa) {
    return false;
}

int delete_constructor::sum() {
    return values[0] + values[1];
}
