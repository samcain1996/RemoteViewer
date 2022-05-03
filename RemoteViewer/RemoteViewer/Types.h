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
#include <array>

#define ONE_BYTE	8
#define TWO_BYTES	(ONE_BYTE + ONE_BYTE)
#define THREE_BYTES (TWO_BYTES + ONE_BYTE)

/*------------------TYPES--------------------*/
using Byte				= unsigned char;

using ByteEncodedUint32 = Byte[4];

using uint32			= std::uint32_t;
using ushort			= std::uint16_t;

using ThreadLock		= std::lock_guard<std::mutex>;

template <typename T>
using ByteArrayMap		= std::unordered_map<uint32, T>;

/*----------------FUNCTIONS--------------------*/

void encode256(ByteEncodedUint32, const uint32);

uint32 decode256(const ByteEncodedUint32);
