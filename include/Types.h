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

#include <mutex>
#include <unordered_map>
#include <SDL.h>

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

enum class Endianess { Little, Big };

struct Difference {
public:
    uint32 _pos;
    uint32 _length;
    ByteArray _sequence;
    Difference(const uint32 begin, const uint32 end, ByteArray seq) {
        _pos = begin;
        _length = end - begin;

        _sequence = new Byte[_length];
        std::memcpy(_sequence, seq, _length);
    }

    Difference() : _pos(0), _length(0), _sequence(nullptr) {}

    Difference(const Difference&) = delete;
    Difference(Difference&&) = delete;

    ~Difference() { if (_sequence) delete[] _sequence; }

private:
    void Copy(const uint32 begin, const uint32 end, ByteArray seq) {
        _sequence = new Byte[_length];
        std::memcpy(_sequence, seq, _length);
    }
public:
    Difference& operator=(const Difference& diff) { Copy(diff._pos, diff._length, diff._sequence); return *this; };

};

struct DiffArray {
private:
    std::pair<Difference*, uint32> _differences;
    uint32& _curIdx = _differences.second;
    const uint32 _capacity;
public:
    DiffArray() = delete;
    DiffArray(const uint32 capacity) : _capacity(capacity), _differences(new Difference[capacity], capacity) {}

    DiffArray(const DiffArray&) = delete;
    DiffArray(DiffArray&&) = delete;

    const std::pair<Difference*, uint32>& Differences() const { return _differences; }
    void AddDifference(const uint32 begin, const uint32 end, ByteArray seq) {
        _differences.first[_curIdx++] = Difference(begin, end, seq);
    }
    void Clear() { _curIdx = 0; }
};

#if defined(__APPLE__)
using DWORD = unsigned int;
#endif
/*----------------FUNCTIONS--------------------*/

constexpr void encode256(ByteEncodedUint32 encodedNumber, const uint32 numberToEncode,
Endianess endianess = Endianess::Big) {
    if (endianess == Endianess::Big) {
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

constexpr uint32 decode256(const ByteEncodedUint32 encodedNumber) {
    return ((uint32)encodedNumber[3] + ((uint32)encodedNumber[2] << ONE_BYTE) +
        ((uint32)encodedNumber[1] << TWO_BYTES) + ((uint32)encodedNumber[0] << THREE_BYTES));
}
