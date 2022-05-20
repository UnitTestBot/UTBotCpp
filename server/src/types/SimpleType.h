/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SIMPLETYPE_H
#define UNITTESTBOT_SIMPLETYPE_H

#include "AbstractType.h"

class SimpleType : public AbstractType {
public:
    enum ReferenceType {
        LValueReference,
        RValueReference,
        NotReference
    };

private:
    uint64_t id;
    bool unnamed;
    bool constQualified;
    ReferenceType referenceType;


public:
    explicit SimpleType(uint64_t id, bool unnamed, bool constQualified,
                        ReferenceType referenceType)
        : id(id), unnamed(unnamed), constQualified(constQualified),
          referenceType(referenceType) {
    }

    [[nodiscard]] Kind getKind() const override {
        return SIMPLE;
    }

    uint64_t getId() const {
        return id;
    }

    bool isUnnamed() const {
        return unnamed;
    }

    bool isConstQualified() const {
        return constQualified;
    }

    bool isReference() const {
        return referenceType != ReferenceType::NotReference;
    }

    bool isLValue() const {
        return referenceType == ReferenceType::LValueReference;
    }

    bool isRValue() const {
        return referenceType == ReferenceType::RValueReference;
    }
};


#endif // UNITTESTBOT_SIMPLETYPE_H
