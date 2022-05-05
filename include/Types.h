#pragma once

/*
*
* Types.h
* 
* Contains commonly used, user-defined type definitions
* and functions
* 
* 
*/
#include <mutex>
#include <unordered_map>

#define ONE_BYTE	8
#define TWO_BYTES	(ONE_BYTE + ONE_BYTE)
#define THREE_BYTES (TWO_BYTES + ONE_BYTE)

/*------------------TYPES--------------------*/
using Byte				= unsigned char;
using ByteArray         = Byte*;
using ByteVec			= std::vector<Byte>;

using ByteEncodedUint32 = Byte[4];

using uint32			= std::uint32_t;
using ushort			= std::uint16_t;

using ThreadLock		= std::lock_guard<std::mutex>;

/*----------------FUNCTIONS--------------------*/

constexpr void encode256(ByteEncodedUint32 encodedNumber, const uint32 numberToEncode) {
    encodedNumber[0] = (Byte)(numberToEncode >> THREE_BYTES) & 0xFF;
    encodedNumber[1] = (Byte)(numberToEncode >> TWO_BYTES) & 0xFF;
    encodedNumber[2] = (Byte)(numberToEncode >> ONE_BYTE) & 0xFF;
    encodedNumber[3] = (Byte)(numberToEncode) & 0xFF;
}

constexpr uint32 decode256(const ByteEncodedUint32 encodedNumber) {
    return ((uint32)encodedNumber[3] + ((uint32)encodedNumber[2] << ONE_BYTE) +
        ((uint32)encodedNumber[1] << TWO_BYTES) + ((uint32)encodedNumber[0] << THREE_BYTES));
}
