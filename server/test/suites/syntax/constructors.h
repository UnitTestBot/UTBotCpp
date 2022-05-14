/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_CONSTRUCTORS_H
#define UNITTESTBOT_CONSTRUCTORS_H


struct BigOrSmallInteger {
    int number;
    BigOrSmallInteger();
    BigOrSmallInteger(bool isBig);
};

struct BufferOfFiveElements {
    int first;
    int last;
    int vec[5];
    BufferOfFiveElements();
    BufferOfFiveElements(int vec[], int size);
};

struct Closet {
    double length;
    double width;
    double height;
    double volume;
    bool flag = false;

    Closet();
    Closet(double length_, double width_, double height_);
    Closet(const Closet &other);
    Closet(Closet &&other);
};

struct Closet2 {
    double length;
    double width;
    double height;
    double volume;

    Closet2();
    Closet2(double length_, double width_, double height_);
    Closet2(const Closet2& other);
    Closet2(Closet2 && other);
};

struct Closet3 {
    double length;
    double width;
    double height;
    double volume;
    bool flag;

    Closet3();
    Closet3(double length_, double width_, double height_);
    Closet3(const Closet3& other);
    Closet3(Closet3 && other);
};

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


#endif // UNITTESTBOT_CONSTRUCTORS_H
