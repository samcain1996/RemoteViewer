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

#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include <memory>
#include <functional>
#include <optional>
#include <thread>
#include <cstring>
#include <array>

using Uint32 = std::uint32_t;
using Ushort = std::uint16_t;

constexpr const Uint32 ONE_BYTE = 8;
constexpr const Uint32 TWO_BYTES = (ONE_BYTE + ONE_BYTE);
constexpr const Uint32 THREE_BYTES = (TWO_BYTES + ONE_BYTE);
constexpr const Uint32 FOUR_BYTES = (TWO_BYTES + TWO_BYTES);

/*------------------TYPES--------------------*/
using Byte				= unsigned char;
using ByteArray         = Byte*;
using BytePtr           = Byte*;

using ByteEncodedUint32 = Byte[FOUR_BYTES];

using ThreadLock		= std::lock_guard<std::mutex>;

enum class Endianess { Little, Big };
constexpr const Endianess DEFAULT_ENDIANESS = Endianess::Little;

#if defined(__APPLE__) || defined(__linux__)
using DWORD = std::uint32_t;
#endif

template <typename Message>
class MessageReader;

template <typename Message>
class MessageWriter;

template <typename Message>
class Messageable {
    friend void ConnectMessageables(Messageable<Message>& m1, Messageable<Message>& m2) {
        m1.msgWriter = new MessageWriter<Message>;
        m2.msgReader = new MessageReader<Message>(m1.msgWriter);

        m2.msgWriter = new MessageWriter<Message>;
        m1.msgReader = new MessageReader<Message>(m2.msgWriter);
    }
public:
    MessageWriter<Message>* msgWriter = nullptr;
    MessageReader<Message>* msgReader = nullptr;

    virtual ~Messageable() {
        if (msgWriter != nullptr) {
            delete msgWriter;
            delete msgReader;
        }
    }
};

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
