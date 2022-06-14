#include "different_parameters.h"

int double_abs(int a) {
    if (a < 0) {
        a *= -2;
    }
    return a;
}

int double_abs_pointer(int* a) {
    if (*a < 0) {
        *a *= -2;
    }
    return (*a);
}

int double_abs_double_pointer(int** a) {
    if (**a < 0) {
        **a *= -2;
    }
    return (**a);
}

int double_abs_lvalue_reference(int& a) {
    if (a < 0) {
        a *= -2;
    }
    return a;
}

// int double_abs_rvalue_reference(int&& a) {
//     if (a < 0) {
//         return -2 * a;
//     }
//     return a;
// }

int double_abs_const(const int a) {
    if (a < 0) {
        return -2 * a;
    }
    return a;
}

int double_abs_const_pointer(const int* a) {
    if (*a < 0) {
        return -2 * (*a);
    }
    return (*a);
}

int double_abs_const_double_pointer(const int** a) {
    if (**a < 0) {
        return -2 * (**a);
    }
    return (**a);
}

int double_abs_const_lvalue_reference(const int& a) {
    if (a < 0) {
        return -2 * a;
    }
    return a;
}

// int double_abs_const_rvalue_reference(const int&& a) {
//     if (a < 0) {
//         return -2 * a;
//     }
//     return a;
// }
