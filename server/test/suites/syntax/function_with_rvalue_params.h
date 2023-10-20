#ifndef UNITTESTBOT_FUNCTION_WITH_RVALUE_PARAMS_H
#define UNITTESTBOT_FUNCTION_WITH_RVALUE_PARAMS_H

struct Box {
    double length;
    double width;
    double height;
    double volume;
    int count;

    Box();

    Box(double length_, double width_, double height_, double volume_, int count);

    int &&return_rvalue_reference();
};

#endif // UNITTESTBOT_FUNCTION_WITH_RVALUE_PARAMS_H
