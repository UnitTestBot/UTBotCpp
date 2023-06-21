#ifndef UTBOTCPP_UNSUPPORTED_CLASS_H
#define UTBOTCPP_UNSUPPORTED_CLASS_H


class private_array {
private:
    int values[2];
public:
    int sum();
};

private_array return_private_array();

bool parameter_private_array(private_array pa);

class delete_constructor {
public:
    delete_constructor() = delete;

    int values[5];

    int sum();
};

class private_constructor {
    private_constructor() = default;

public:
    int values[2];

    int sum();
};


#endif //UTBOTCPP_UNSUPPORTED_CLASS_H
