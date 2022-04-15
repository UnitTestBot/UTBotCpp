//
// Created by mWX1153800 on 4/22/2022.
//

#ifndef UNITTESTBOT_STRUCT_WITH_PARAMETRIZED_CONSTRUCTOR_H
#define UNITTESTBOT_STRUCT_WITH_PARAMETRIZED_CONSTRUCTOR_H
#include <cassert>

struct Closet4 {
    double length;
    double width;
    double height;
    double volume;

    Closet4();
    Closet4(double length_, double width_, double height_);
    Closet4(const Closet4& other);
    Closet4(Closet4 && other);
};
#endif //UNITTESTBOT_STRUCT_WITH_PARAMETRIZED_CONSTRUCTOR_H
