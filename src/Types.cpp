#include "Types.h"

void encode256(ByteEncodedUint32 encodedNumber, const uint32 numberToEncode) {
    encodedNumber[0] = (Byte)(numberToEncode >> THREE_BYTES)   & 0xFF;
    encodedNumber[1] = (Byte)(numberToEncode >> TWO_BYTES)     & 0xFF;
    encodedNumber[2] = (Byte)(numberToEncode >> ONE_BYTE)      & 0xFF;
    encodedNumber[3] = (Byte)(numberToEncode)                  & 0xFF;
}

uint32 decode256(const ByteEncodedUint32 encodedNumber) {
    return ((uint32)encodedNumber[3]               + ((uint32)encodedNumber[2] << ONE_BYTE) +
           ((uint32)encodedNumber[1] << TWO_BYTES) + ((uint32)encodedNumber[0] << THREE_BYTES));
}