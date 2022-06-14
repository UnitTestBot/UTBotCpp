#ifndef UNITTESTBOT_ARRAYTYPE_H
#define UNITTESTBOT_ARRAYTYPE_H

#include "AbstractType.h"

class ArrayType : public AbstractType {
public:
    explicit ArrayType(unsigned long int size, bool complete) : size(size), complete(complete) {}

    [[nodiscard]] unsigned long int getSize() const override {
        return size;
    }

    [[nodiscard]] Kind getKind() const override {
        return ARRAY;
    }

    bool isComplete() const {
        return complete;
    }
private:
    unsigned long int size;
    bool complete;
};

#endif
