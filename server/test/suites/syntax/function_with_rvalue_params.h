#ifndef UNITTESTBOT_FUNCTION_WITH_RVALUE_PARAMS_H
#define UNITTESTBOT_FUNCTION_WITH_RVALUE_PARAMS_H

struct Closet {
    double length;
    double width;
    double height;
    double volume;

    Closet();
    Closet(double length_, double width_, double height_, double volume_);
};

int double_max(int && first, int && second);

int remainder(const int && value);

int && return_and_get_rvalue_reference(int && first, int && second);

int get_rvalue_custom_struct_as_param(Closet && closet);

#endif // UNITTESTBOT_FUNCTION_WITH_RVALUE_PARAMS_H
