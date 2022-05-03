#include "Types.h"

void encode256(ByteEncodedUint32 encodedNumber, const uint32 numberToEncode) {
    encodedNumber[0] = (unsigned char)(numberToEncode >> THREE_BYTES)   & 0xFF;
    encodedNumber[1] = (unsigned char)(numberToEncode >> TWO_BYTES)     & 0xFF;
    encodedNumber[2] = (unsigned char)(numberToEncode >> ONE_BYTE)      & 0xFF;
    encodedNumber[3] = (unsigned char)(numberToEncode)                  & 0xFF;
}

uint32 decode256(const ByteEncodedUint32 encodedNumber) {
    return ((unsigned int)encodedNumber[3]               + ((unsigned int)encodedNumber[2] << ONE_BYTE) +
           ((unsigned int)encodedNumber[1] << TWO_BYTES) + ((unsigned int)encodedNumber[0] << THREE_BYTES));
}