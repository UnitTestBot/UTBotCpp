/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "VirtualMethods.h"

Animal::Animal() : type("Animal") {}

string Animal::getType() {
    return type;
}

Dog::Dog() : type("Dog") {}

string Dog::getType() {
    return type;
}

Cat::Cat() : type("Cat") {}

string Cat::getType() {
    return type;
}

string concatTypes() {
    auto* animal1 = new Animal();
    Animal* dog1 = new Dog();
    Animal* cat1 = new Cat();
    return animal1->getType() + " " + dog1->getType() + " " + cat1->getType();
}
