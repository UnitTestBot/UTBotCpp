#include <utility>
#include "function_with_rvalue_params.h"

int double_max(int && first, int && second) {
    if (first > second) {
        return 2 * first;
    } else {
        return 2 * second;
    }
}

int remainder(const int && value) {
    if (value % 3 == 0) {
        return 0;
    } else if (value % 3 == 1) {
        return 1;
    } else {
        return 2;
    }
}

int && return_and_get_rvalue_reference(int && first, int && second) {
    if (first % 5 == 0) {
        return std::move(first);
    } else if (second % 5 == 0) {
        return std::move(second);
    } else {
        return std::move(first + second);
    }
}

int get_rvalue_custom_struct_as_param(Closet && closet) {
    if (closet.height > 5 && closet.width > 5 && closet.length > 5) {
        closet.height /= 5;
        closet.volume /= 5;
        closet.width /= 5;
        return 1;
    } else {
        closet.width = 5;
        closet.height = 5;
        closet.length = 5;
        closet.volume = 125;
        return 2;
    }
}

Closet::Closet() {
    length = 1.5;
    width = 0.5;
    height = 2.5;
    volume = height * width * length;
}

Closet::Closet(double length_, double width_, double height_, double volume_) {
    length = length_;
    width = width_;
    height = height_;
    volume = volume_;
}
