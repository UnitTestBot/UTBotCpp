#ifndef SIMPLE_TEST_PROJECT_STRUCTS_WITH_POINTERS_H
#define SIMPLE_TEST_PROJECT_STRUCTS_WITH_POINTERS_H


struct StructWithCharPointer {
    int a;
    char * str;
};


struct List {
    struct List * next;
    int val;
};

struct StructWithPointerInField {
    long long ll;
    unsigned short sh;
    struct StructWithCharPointer a;
};

int list_sum(struct List* head);

int handle_struct_with_char_ptr(struct StructWithCharPointer st);

int list_sum_sign(struct List *head);

int unsafe_next_value(struct List *node);

long long sum_of_all_fields_or_mult(struct StructWithPointerInField st, int a);

struct StructWithPointerInField some_calc(int a, int b);

struct StructWithPointer {
  int x;
  int* y;
};

struct StructWithDoublePointer {
  int x;
  int** y;
};

struct StructWithArrayOfPointer {
  int x;
  int* y[2];
};

struct StructWithStructWithPointer {
  struct StructWithPointer swp;
  struct StructWithDoublePointer* swdp;
};

struct StructManyPointers {
  int a;
  int* b;
  int** c;
  int*** d;
};

struct StructComplex {
  int x;
  int* y;
  int** z;
  struct StructWithPointer* swp;
  struct StructWithDoublePointer** swdp;
  struct StructManyPointers smp;
};

int sumStructWithPointer(struct StructWithPointer par);
int sumStructWithPointerAsPointer(struct StructWithPointer* par);
int sumStructWithDoublePointer(struct StructWithDoublePointer par);
int sumStructWithArrayOfPointer(struct StructWithArrayOfPointer par);
int sumStructWithStructWithPointer(struct StructWithStructWithPointer par);
int sumStructManyPointers(struct StructManyPointers par);
int sumStructComplex(struct StructComplex par);
#endif // SIMPLE_TEST_PROJECT_STRUCTS_WITH_POINTERS_H
