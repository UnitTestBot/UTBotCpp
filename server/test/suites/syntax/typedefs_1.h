#ifndef SIMPLE_TEST_PROJECT_CHECK_TYPEDEF_1_H
#define SIMPLE_TEST_PROJECT_CHECK_TYPEDEF_1_H
#include <sys/types.h>
#include <stddef.h>

int gid_to_name (gid_t gid);

typedef struct {
    int a;
} TypeDefStruct;

typedef struct __typeDefStruct {
    int a;
} TypeDefStruct2;

typedef size_t size_t_alias;

int sign_of_typedef_struct(TypeDefStruct x);

int sign_of_typedef_struct2(TypeDefStruct2 x);

size_t min_size_t(size_t a, size_t b);

size_t_alias min_size_t_alias(size_t_alias a, size_t_alias b);

typedef union {
    int a;
} TypeDefUnion;

typedef union _typeDefUnion {
    int a;
} TypeDefUnion2;

int sign_of_typedef_union(TypeDefUnion x);

int sign_of_typedef_union2(TypeDefUnion2 x);

#endif
