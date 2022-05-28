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

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <vector>
#include <mutex>
#include <queue>

constexpr const Uint32 ONE_BYTE    = 8;
constexpr const Uint32 TWO_BYTES   = (ONE_BYTE + ONE_BYTE);
constexpr const Uint32 THREE_BYTES = (TWO_BYTES + ONE_BYTE);
constexpr const Uint32 FOUR_BYTES  = (TWO_BYTES + TWO_BYTES);

/*------------------TYPES--------------------*/
using Byte				= unsigned char;
using ByteArray         = Byte*;
using ByteVec			= std::vector<Byte>;

using ByteEncodedUint32 = Byte[FOUR_BYTES];

using Ushort			= std::uint16_t;

using ThreadLock		= std::lock_guard<std::mutex>;

enum class Endianess { Little, Big };
constexpr const Endianess DEFAULT_ENDIANESS = Endianess::Little;

#if defined(__APPLE__) || defined(__linux__)
using DWORD = std::uint32_t;
#endif
/*----------------FUNCTIONS--------------------*/

constexpr void encode256(ByteEncodedUint32 encodedNumber, const Uint32 numberToEncode,
    const Endianess endianess = DEFAULT_ENDIANESS) {
    if (endianess == Endianess::Little) {
        encodedNumber[0] = (Byte)(numberToEncode >> THREE_BYTES) & 0xFF;
        encodedNumber[1] = (Byte)(numberToEncode >> TWO_BYTES) & 0xFF;
        encodedNumber[2] = (Byte)(numberToEncode >> ONE_BYTE) & 0xFF;
        encodedNumber[3] = (Byte)(numberToEncode) & 0xFF;
    }
    else {
        encodedNumber[3] = (Byte)(numberToEncode >> THREE_BYTES) & 0xFF;
        encodedNumber[2] = (Byte)(numberToEncode >> TWO_BYTES) & 0xFF;
        encodedNumber[1] = (Byte)(numberToEncode >> ONE_BYTE) & 0xFF;
        encodedNumber[0] = (Byte)(numberToEncode) & 0xFF; 
    }
}

constexpr Uint32 decode256(const ByteEncodedUint32 encodedNumber, const Endianess endianess = DEFAULT_ENDIANESS) {
    if (endianess == Endianess::Big) {
        return ((Uint32)encodedNumber[0] + ((Uint32)encodedNumber[1] << ONE_BYTE) +
            ((Uint32)encodedNumber[2] << TWO_BYTES) + ((Uint32)encodedNumber[3] << THREE_BYTES));
    }
    else {
        return ((Uint32)encodedNumber[3] + ((Uint32)encodedNumber[2] << ONE_BYTE) +
            ((Uint32)encodedNumber[1] << TWO_BYTES) + ((Uint32)encodedNumber[0] << THREE_BYTES));
    }
}

constexpr const Uint32 CalculateTheoreticalBMPSize(const Uint32 width, const Uint32 height) {
    return ((width * 32 + 31) / 32) * 4 * height;
}
