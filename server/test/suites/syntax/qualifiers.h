#include <string.h>

struct MinMaxQ {
    int a;
    int b;
    char chars[3];
};

int c_strcmp_2(const char * restrict const a, const char * restrict const b);

int ishello_2(char * restrict a);

const long long * const returns_pointer_with_min_modifier(const long long a, const long long b);

char * const foo__(int const a);

const char * const foo_bar(int a);

const struct MinMaxQ * restrict const returns_struct_with_min_max_Q(volatile const int a, const volatile int b);
