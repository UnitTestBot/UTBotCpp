#ifndef CPP_EXAMPLE_BITFIELDS_H
#define CPP_EXAMPLE_BITFIELDS_H

struct CppSpecificStruct {
    // will **usually** occupy 40 bytes:
    // 7  bits: value of b1
    // 57 bits: unused
    // 64 bits: value of breaking
    // 17 bits: value of b2
    // 1  bit:  value of b3
    // 22 bits: value of b4
    // 24 bits: unused
    // 32 bits: value of b5
    // 34 bits: unused
    // 4  bits: value of b6
    // 58 bits: unused
    unsigned b1 : 7;    // from 0 to 127
    long long breaking; // from LLONG_MIN to LLONG_MAX
    signed b2 : 17;     // from -65536 to 65535
    bool b3 : 1;        // from 0 to 1
    int b4 : 22;        // from -2097152 to 2097151
    unsigned : 0;       // starts a new allocation unit
    signed b5 : 66;     // from INT_MIN to INT_MAX
    signed b6 : 4;      // from -8 to 7
};

CppSpecificStruct modify_and_return_copy(CppSpecificStruct& s);

#endif //CPP_EXAMPLE_BITFIELDS_H
