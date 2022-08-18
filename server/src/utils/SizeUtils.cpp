#include "SizeUtils.h"

size_t SizeUtils::bytesToBits(size_t bytes) {
    return bytes * CHAR_BIT;
}

size_t SizeUtils::bitsToBytes(size_t bits) {
    return bits / CHAR_BIT;
}
