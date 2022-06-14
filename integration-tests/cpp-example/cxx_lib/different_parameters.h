#ifndef UNITTESTBOT_DIFFERENT_PARAMETERS_H
#define UNITTESTBOT_DIFFERENT_PARAMETERS_H

int double_abs(int a);
int double_abs_pointer(int* a);
int double_abs_double_pointer(int** a);
int double_abs_lvalue_reference(int& a);
// int double_abs_rvalue_reference(int&& a);
int double_abs_const(const int a);
int double_abs_const_pointer(const int* a);
int double_abs_const_double_pointer(const int** a);
int double_abs_const_lvalue_reference(const int& a);
// int double_abs_const_rvalue_reference(const int&& a);


#endif //UNITTESTBOT_DIFFERENT_PARAMETERS_H
