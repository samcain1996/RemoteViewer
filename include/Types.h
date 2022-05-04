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
using ByteVec			= std::vector<Byte>;

using ByteEncodedUint32 = Byte[4];

using uint32			= std::uint32_t;
using ushort			= std::uint16_t;

using ThreadLock		= std::lock_guard<std::mutex>;

/*----------------FUNCTIONS--------------------*/

void encode256(ByteEncodedUint32, const uint32);

uint32 decode256(const ByteEncodedUint32);
