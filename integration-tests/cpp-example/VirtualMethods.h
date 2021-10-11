/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef CPP_PROJECT_VIRTUALMETHODS_H
#define CPP_PROJECT_VIRTUALMETHODS_H

#include <iostream>
#include <string>
using namespace std;

class Animal {
private:
    string type;

public:
    Animal();

    virtual string getType();
};

class Dog : public Animal {
private:
    string type;

public:
    Dog();

    string getType() override;
};

class Cat : public Animal {
private:
    string type;

public:
    Cat();

    string getType() override;
};

string concatTypes();

#endif //CPP_PROJECT_VIRTUALMETHODS_H
