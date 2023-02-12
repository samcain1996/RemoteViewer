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

#include <span>
#include <mutex>
#include <queue>
#include <array>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <cstring>
#include <optional>
#include <functional>

#include "Logger/Logger.h"


#if defined(__APPLE__) || defined(__linux__)

using DWORD = std::uint32_t;

#endif

enum class OPERATING_SYSTEM : char {
    WINDOWS = 'W',
    LINUX = 'L',
    MAC = 'M',
    NA = '\0'
};

using Uint32 = std::uint32_t;
using Ushort = std::uint16_t;

constexpr const Uint32 ONE_BYTE = 8;
constexpr const Uint32 TWO_BYTES = (ONE_BYTE + ONE_BYTE);
constexpr const Uint32 THREE_BYTES = (TWO_BYTES + ONE_BYTE);
constexpr const Uint32 FOUR_BYTES = (TWO_BYTES + TWO_BYTES);

/*------------------TYPES--------------------*/
using MyByte = unsigned char;
using BytePtr = MyByte*;

constexpr const MyByte MAX_MYBYTE_VAL = static_cast<MyByte>(255);

using ThreadLock = std::lock_guard<std::mutex>;

/*----------------FUNCTIONS--------------------*/

constexpr void EncodeAsByte(MyByte encodedNumber[4], const Uint32 numberToEncode) {

    encodedNumber[3] = (MyByte)(numberToEncode >> 24) & 0xFF;
    encodedNumber[2] = (MyByte)(numberToEncode >> 16) & 0xFF;
    encodedNumber[1] = (MyByte)(numberToEncode >> 8) & 0xFF;
    encodedNumber[0] = (MyByte)(numberToEncode) & 0xFF;

}

constexpr Uint32 DecodeAsByte(const MyByte encodedNumber[4]) {

    return ((Uint32)encodedNumber[0] + ((Uint32)encodedNumber[1] << ONE_BYTE) +
        ((Uint32)encodedNumber[2] << TWO_BYTES) + ((Uint32)encodedNumber[3] << THREE_BYTES));

}

using Action = std::function<void()>;
