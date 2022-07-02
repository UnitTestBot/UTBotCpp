#include "exceptions.h"

#include <stdexcept>
#include <exception>
#include <new>
#include <typeinfo>
#include <optional>
#include <variant>

int stdException(int a) {
    if (a > 5) {
        throw std::exception();
    } else {
        return 5;
    }
}

int badAllocException(int a) {
    if (a > 5) {
        throw std::bad_alloc();
    } else {
        return 5;
    }
}

int badCastException(int a) {
    if (a > 5) {
        throw std::bad_cast();
    } else {
        return 5;
    }
}

int optionalException(int a) {
    if (a > 5) {
        throw std::bad_optional_access();
    } else {
        return 5;
    }
}

int variantException(int a) {
    if (a > 5) {
        throw std::bad_variant_access();
    } else {
        return 5;
    }
}

int customException(int a) {
    if (a > 5) {
        throw MyException();
    } else {
        return 5;
    }
}

