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
#include <memory>
#include <functional>
#include <thread>
#include <cstring>
#include <array>
#include <cmath>
#include <iostream>

using Uint32 = std::uint32_t;
using Ushort = std::uint16_t;

/*------------------TYPES--------------------*/
using Byte				= unsigned char;
using ByteVec           = std::vector<Byte>;

using ThreadLock		= std::lock_guard<std::mutex>;

#if defined(__APPLE__) || defined(__linux__)
using DWORD = std::uint32_t;
#endif

#if defined(_WIN32)
constexpr const bool IS_WINDOWS = true;
#else
constexpr const bool IS_WINDOWS = false;
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

constexpr void EncodeAsByte(Byte encodedNumber[4], const Uint32 numberToEncode) {

    encodedNumber[3] = (Byte)(numberToEncode >> 24) & 0xFF;
    encodedNumber[2] = (Byte)(numberToEncode >> 16) & 0xFF;
    encodedNumber[1] = (Byte)(numberToEncode >> 8) & 0xFF;
    encodedNumber[0] = (Byte)(numberToEncode) & 0xFF;

}

constexpr Uint32 DecodeAsByte(const Byte encodedNumber[4]) {

    return ((Uint32)encodedNumber[0] + ((Uint32)encodedNumber[1] << 8) +
        ((Uint32)encodedNumber[2] << 16) + ((Uint32)encodedNumber[3] << 24));

}
