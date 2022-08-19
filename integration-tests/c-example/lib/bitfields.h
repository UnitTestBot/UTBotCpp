#ifndef C_EXAMPLE_BITFIELDS_H
#define C_EXAMPLE_BITFIELDS_H

#include <stdbool.h>

typedef struct {
    signed a : 24;
    signed b : 1;
    signed int c : 2;
    signed int d : 5;
} SimpleSignedStr;

typedef struct {
    unsigned a : 2;
    unsigned b : 5;
    unsigned int c : 1;
    unsigned int d : 24;
} SimpleUnsignedStr;

typedef struct {
    int a : 24;
    int b : 2;
    int c : 1;
    int d : 5;
} ImplementationDefinedStr;

typedef struct {
    signed a : 3;
    signed b : 3;
} PossiblySmallStr;

typedef union {
    unsigned a : 2;
    unsigned b : 1;
    unsigned c : 6;
    unsigned d : 23;
} SimpleUnsignedUnion;

typedef struct {
    // will **usually** occupy 8 bytes:
    bool a : 1;
    unsigned b : 2;
    signed c : 1;
    unsigned d : 3;
    // 25 bits: unused
    int e;
} ComplexStr;

typedef struct {
    // will **usually** occupy 1 byte
    bool a : 1, b : 1, c : 1;
} StrWithBool;

// struct InvalidNumberOfBits { // should produce an error in C
//     bool a : 2;
//     unsigned b : 50;
//     signed c : 1;
//     unsigned d : 3;
// };

typedef struct {
    // will **usually** occupy 4 bytes:
    // 5 bits: value of b1
    // 11 bits: unused -- explicitly specified padding
    // 6 bits: value of b2
    // 2 bits: value of b3
    // 3 bits: unused -- explicitly specified padding
    // 5 bits: unused -- implicitly
    unsigned b1 : 5, : 11, b2 : 6, b3 : 2;
    signed : 3;
} StrWithUnnamedBitfields;

typedef struct {
    // will **usually** occupy 8 bytes:
    // 7 bits: value of b1
    // 25 bits: unused
    // 6 bits: value of b2
    // 15 bits: value of b3
    // 11 bits: unused
    unsigned b1 : 7;
    unsigned : 0; // start a new allocation unit
    unsigned b2 : 6;
    unsigned b3 : 15;
} StrWithUnnamedZeroBitfield;

typedef struct {
    // will **usually** occupy 24 bytes:
    // 7 bits: value of b1
    // 57 bits: unused
    // 64 bits: value of breaking
    // 17 bits: value of b2
    // 1 bit: value of b3
    // 22 bits: value of b4
    // 24 bits: unused
    unsigned b1 : 7;    // from 0 to 127
    long long breaking; // from LLONG_MIN to LLONG_MAX
    signed b2 : 17;     // from -65536 to 65535
    bool b3 : 1;        // from 0 to 1
    int b4 : 22;        // usually from -2097152 to 2097151
} StrWithBreak;


#endif //C_EXAMPLE_BITFIELDS_H
