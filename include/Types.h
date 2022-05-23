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

//struct Difference {
//public:
//    Uint32 _pos;
//    Uint32 _length;
//    ByteArray _sequence;
//    Difference(const Uint32 begin, const Uint32 end, ByteArray seq) {
//        _pos = begin;
//        _length = end - begin;
//
//        _sequence = new Byte[_length];
//        std::memcpy(_sequence, seq, _length);
//    }
//
//    Difference() : _pos(0), _length(0), _sequence(nullptr) {}
//
//    Difference(const Difference&) = delete;
//    Difference(Difference&&) = delete;
//
//    ~Difference() { if (_sequence) delete[] _sequence; }
//
//private:
//    void Copy(const Uint32 begin, const Uint32 end, ByteArray seq) {
//        _sequence = new Byte[_length];
//        std::memcpy(_sequence, seq, _length);
//    }
//public:
//    Difference& operator=(const Difference& diff) { Copy(diff._pos, diff._length, diff._sequence); return *this; };
//
//};
//
//struct DiffArray {
//private:
//    std::pair<Difference*, Uint32> _differences;
//    Uint32& _curIdx = _differences.second;
//    const Uint32 _capacity;
//public:
//    DiffArray() = delete;
//    DiffArray(const Uint32 capacity) : _capacity(capacity), _differences(new Difference[capacity], capacity) {}
//
//    DiffArray(const DiffArray&) = delete;
//    DiffArray(DiffArray&&) = delete;
//
//    const std::pair<Difference*, Uint32>& Differences() const { return _differences; }
//    void AddDifference(const Uint32 begin, const Uint32 end, ByteArray seq) {
//        _differences.first[_curIdx++] = Difference(begin, end, seq);
//    }
//    void Clear() { _curIdx = 0; }
//};

#if defined(__APPLE__)
using DWORD = unsigned int;
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
