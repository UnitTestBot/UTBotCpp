#ifndef UNITTESTBOT_CONSTRUCTORS_H
#define UNITTESTBOT_CONSTRUCTORS_H


struct BigOrSmallInteger {
    int number;

    BigOrSmallInteger();

    BigOrSmallInteger(bool isBig);
};

struct TwoElements {
    int first;
    int second;

    TwoElements();

    TwoElements(int *first_, int *second_);
};

struct Closet {
    double length;
    double width;
    double height;
    double volume;

    Closet();

    Closet(double length_, double width_, double height_, double volume_);

    Closet(const Closet &other);
};

struct Closet2 {
    double length;
    double width;
    double height;
    double volume;

    Closet2();
};

struct Closet3 {
    double length;
    double width;
    double height;
    double volume;

    Closet3();

    Closet3(double length_, double width_, double height_, double volume_);

    Closet3(Closet3 &&other);
};

struct Closet4 {
    double length;
    double width;
    double height;
    double volume;

    Closet4();

    Closet4(double length_, double width_, double height_);
};


#endif // UNITTESTBOT_CONSTRUCTORS_H
