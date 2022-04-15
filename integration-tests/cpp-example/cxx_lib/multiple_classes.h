#ifndef UNITTESTBOT_MULTIPLE_CLASSES_H
#define UNITTESTBOT_MULTIPLE_CLASSES_H


class first_class {
public:
    first_class();

    int get1();
};

struct second_class {
public:
    second_class();

    struct third_class {
        third_class();

    public:
        int get3();
    };

    int get2();
};


#endif //UNITTESTBOT_MULTIPLE_CLASSES_H
