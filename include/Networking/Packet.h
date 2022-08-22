#pragma once
#include <unordered_map>
#include <boost/container/static_vector.hpp>
#include "Types.h"
#include "Capture.h"

// Defines related to packets

constexpr const Uint32 MAX_PACKET_SIZE					= 4096;

constexpr const Uint32 PACKET_HEADER_COUNT				= 3;
constexpr const Uint32 PACKET_HEADER_ELEMENT_SIZE		= 4;
constexpr const Uint32 PACKET_HEADER_SIZE				= (PACKET_HEADER_ELEMENT_SIZE * PACKET_HEADER_COUNT);

constexpr const Uint32 MAX_PACKET_PAYLOAD_SIZE			= (MAX_PACKET_SIZE - PACKET_HEADER_SIZE);

constexpr const Uint32 PACKET_ENCODED_OFFSET			= 0;
constexpr const Uint32 PACKET_GROUP_OFFSET				= (PACKET_HEADER_ELEMENT_SIZE);
constexpr const Uint32 PACKET_SEQUENCE_OFFSET			= (PACKET_HEADER_ELEMENT_SIZE * 2);
constexpr const Uint32 PACKET_PAYLOAD_OFFSET			= PACKET_HEADER_SIZE;

using PacketGroup    = Uint32;
using PacketPayload  = boost::container::static_vector<Byte, MAX_PACKET_PAYLOAD_SIZE>;
using PacketBuffer   = std::array<Byte, MAX_PACKET_SIZE>;
using PacketMetadata = std::array<Byte, PACKET_HEADER_SIZE>;

enum class PacketTypes : Byte {
	Image = static_cast<Byte>('I'),
	Invalid = static_cast<Byte>('\0')
};

// Holds _metadata about a packet
struct PacketHeader {
	static const Ushort SIZE_OFFSET = 1;
	
	PacketMetadata _metadata;
	const Uint32 Size() const;

	virtual const PacketTypes PacketType() const { return PacketTypes::Invalid; }

	PacketHeader(const PacketBuffer& packetBuffer);

	PacketHeader(const PacketHeader& header);

	PacketHeader(const PacketPayload& payload, const PacketMetadata& metadata);

	virtual ~PacketHeader() {}
	
	
};

struct ImagePacketHeader : private PacketHeader {
	static const Ushort POSITION_OFFSET = SIZE_OFFSET + 4;

	const Uint32 Position() const;
	const Uint32 Size() const;

	const PacketTypes PacketType() const override { return PacketTypes::Image; }

	ImagePacketHeader(const PacketHeader& header) : PacketHeader(header) {}
};

// Packet of data that can be sent over a socket
class Packet {

public:
	
	static const bool InvalidImagePacket(const PacketBuffer& packetBuffer);
	static const Uint32 DecodeAsByte(const Byte encodedNumber[4]);
	
private:
	PacketHeader  _header;		// Header containing packet _metadata
	PacketPayload _payload;		// Packet data
public:

	/*-----------------PACKET--------------------*/
	/*											 */	
	/*  Packets should only be constructed from  */
	/*		1. Another packet					 */
	/*		2. A header and payload				 */
	/*											 */
	/*-------------------------------------------*/
	Packet() = delete;

	Packet(const Packet&);
	Packet(Packet&&) noexcept;
	
	Packet(const PacketBuffer& packetData);
	Packet(const PacketHeader& header, const PacketPayload& payload);

	Packet& operator=(const Packet&);
	Packet& operator=(Packet&&) noexcept;

	const PacketBuffer	    RawData() const;  // _metadata and payload in a contiguous array
	const PacketHeader	    Header()  const;  // _metadata
	const PacketPayload		Payload() const;  // Payload
	// const Ushort			Size()    const;  // Size of the whole packet

};

using PacketList			      = std::vector<Packet>;