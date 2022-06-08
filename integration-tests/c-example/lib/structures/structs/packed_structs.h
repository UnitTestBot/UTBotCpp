#ifndef SIMPLE_TEST_PACKED_STRUCTS_H
#define SIMPLE_TEST_PACKED_STRUCTS_H

#pragma pack(push, 1)
struct PackedStruct1 {
    short s;
    int i;
};
#pragma pack(pop)

struct myUnpackedStruct {
    char c;
    int i;
};
    
struct __attribute__ ((__packed__)) PackedStruct2 {
    char c;
    int  i;
    struct myUnpackedStruct s;
};

#pragma pack(push, 8)
struct OtherPackedStruct {
    char a;
    struct PackedStruct2 str;
    char b;
    short s;
}; 
#pragma pack(pop)

int get_sign_packedStruct1(struct PackedStruct1 st);

char get_val_by_packedStruct2(struct PackedStruct2 st);

short get_val_by_otherPackedStruct(struct OtherPackedStruct st);

#endif //SIMPLE_TEST_PACKED_STRUCTS_H
