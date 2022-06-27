#include <stdlib.h>

struct UnnamedTypeUnionField
{
  int count;
  union
  {
    int i;
    char c[4];
  } value;
};

int extract_value(struct UnnamedTypeUnionField arg) {
    if (arg.count > 0) {
        return arg.value.i;
    }
    return 0;
}

union UnnamedTypeStructField {
    struct {
        int a;
        double b;
    } value1;  
    struct {
        char c;
        int d;
    } value2;  
};

int extract_value_2(union UnnamedTypeStructField arg) {
    if (arg.value2.c == 0) {
        return arg.value1.a;
    }
    return arg.value2.d;
}

struct AnonymousUnionField {
    union { // anonymous union
        int i;
        char c[4];
    };
};

int extract_value_3(struct AnonymousUnionField arg) {
    if (arg.i > 0) {
        return arg.i;
    }
    return (int) arg.c[0];
}

union AnonymousStructField {
    struct {
        int a;
        double b;
    };  
    struct {
        char c;
        int d;
    };  
};

int extract_value_4(union AnonymousStructField arg) {
    if (arg.c == 0) {
        return arg.a;
    }
    return arg.d;
}

struct Vector {
    int len;
    int data[];
};

long sum_of(struct Vector arg) {
    long sum = 0;
    for (int i = 0; i < arg.len; i++) {
        sum += arg.data[i];
    }
    if (sum > 0) {
        return sum;
    }
    if (sum < 0) {
        return -sum;
    }
    return 0;
}

static struct Vector* create(int len) {
    struct Vector *vector = malloc(sizeof(struct Vector) + sizeof(int) * len);
    vector->len = len;
    for (int i = 0; i < vector->len; i++) {
        vector->data[i] = 0;
    }
    return vector;
}

struct IncompleteType;

void accept_incomplete(struct IncompleteType *arg) {
    free(arg);
}

struct IncompleteType* return_incomplete() {
    void *ptr = malloc(100);
    return (struct IncompleteType *)(ptr);
}

struct ForwardDecl;

struct ForwardDecl* pass_forward_decl(struct ForwardDecl* arg) {
    return arg;
}

struct ForwardDecl {
    int x, y;
};

// it's a duplicate of struct in types.h
struct SupportedStruct1 {
    int length;
    int x[2];
};

long mul_of(struct SupportedStruct1 arg) {
    long mul = 1;
    for (int i = 0; i < arg.length; i++) {
        mul *= arg.x[i];
    }
    if (mul < 0) {
        return -mul;
    }
    return mul;
}

enum {
    FALSE,
    TRUE
} option;

_Bool check_option() {
    if (option == FALSE) {
        return -1;
    }
    return +1;
}
