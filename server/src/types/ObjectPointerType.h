#ifndef UNITTESTBOT_OBJECTPOINTERTYPE_H
#define UNITTESTBOT_OBJECTPOINTERTYPE_H

#include "AbstractType.h"

class ObjectPointerType : public AbstractType {
    bool constQualified;

public:
    explicit ObjectPointerType(bool constQualified) : constQualified(constQualified) {
    }

    [[nodiscard]] Kind getKind() const override {
        return OBJECT_POINTER;
    }

    bool isConstQualified() const {
        return constQualified;
    }
};
#endif
