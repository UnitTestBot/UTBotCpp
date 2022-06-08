#ifndef UNITTESTBOT_FUNCTION_POINTERTYPE_H
#define UNITTESTBOT_FUNCTION_POINTERTYPE_H

#include "AbstractType.h"

class FunctionPointerType: public AbstractType {
public:
    [[nodiscard]] Kind getKind() const override {
        return FUNCTION_POINTER;
    }
};
#endif
