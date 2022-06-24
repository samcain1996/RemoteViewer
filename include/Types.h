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

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <mutex>
#include <queue>
#include <cstring>
#include <atomic>
#include <memory>
#include <functional>
#include <optional>

constexpr const Uint32 ONE_BYTE    = 8;
constexpr const Uint32 TWO_BYTES   = (ONE_BYTE + ONE_BYTE);
constexpr const Uint32 THREE_BYTES = (TWO_BYTES + ONE_BYTE);
constexpr const Uint32 FOUR_BYTES  = (TWO_BYTES + TWO_BYTES);


/*------------------TYPES--------------------*/
using Byte				= unsigned char;
using ByteArray         = Byte*;
using BytePtr           = Byte*;

using ByteEncodedUint32 = Byte[FOUR_BYTES];

using Ushort			= std::uint16_t;

using ThreadLock		= std::lock_guard<std::mutex>;

template <typename T>
using Validator = std::function<bool(T)>;

enum class Endianess { Little, Big };
constexpr const Endianess DEFAULT_ENDIANESS = Endianess::Little;

constexpr const Ushort MillisInSecs = 1000;

constexpr const SDL_Color WHITE = { 255, 255, 255 };
constexpr const SDL_Color GREEN = { 0,255,0 };
constexpr const SDL_Color PINK = { 255,0,255 };
constexpr const SDL_Color BLACK = { 0,0,0 };

#if defined(__APPLE__) || defined(__linux__)
using DWORD = std::uint32_t;
#endif

template <typename Message>
class MessageReader;

template <typename Message>
class MessageWriter;

template <typename Message>
class Messageable {
    friend class Application;
protected:
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

constexpr const Uint32 CalculateTheoreticalBMPSize(const Uint32 width, const Uint32 height) {
    return ((width * 32 + 31) / 32) * 4 * height;
}


static const Validator<const char> NUMERIC_VALIDATOR = [](const char c) {
	return (c >= '0' && c <= '9');
};

static const Validator<const char> ALPHABETIC_VALIDAOTR = [](const char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
};

static const Validator<const char> ALPHANUMERIC_VALIDATOR = [](const char c) {
	return NUMERIC_VALIDATOR(c) || ALPHABETIC_VALIDAOTR(c);
};

static const Validator<const char> IP_VALIDATOR = [](const char c) {
	return (c >= '0' && c <= '9') || (c == '.');
};

template <typename T>
using ValidatorList = std::vector<Validator<T>>;