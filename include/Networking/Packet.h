#pragma once
#include <unordered_map>
#include <boost/container/static_vector.hpp>
#include "Types.h"
#include "Capture.h"

// Defines related to packets

constexpr const Uint32 MAX_PACKET_SIZE = 4096;

constexpr const Uint32 PACKET_HEADER_COUNT = 3;
constexpr const Uint32 PACKET_HEADER_ELEMENT_SIZE = 4;
constexpr const Uint32 PACKET_HEADER_SIZE = (PACKET_HEADER_ELEMENT_SIZE * PACKET_HEADER_COUNT);

constexpr const Uint32 MAX_PACKET_PAYLOAD_SIZE = (MAX_PACKET_SIZE - PACKET_HEADER_SIZE);

constexpr const Uint32 PACKET_ENCODED_OFFSET = 0;
constexpr const Uint32 PACKET_GROUP_OFFSET = (PACKET_HEADER_ELEMENT_SIZE);
constexpr const Uint32 PACKET_SEQUENCE_OFFSET = (PACKET_HEADER_ELEMENT_SIZE * 2);
constexpr const Uint32 PACKET_PAYLOAD_OFFSET = PACKET_HEADER_SIZE;

using PacketGroup = Uint32;
using PacketPayload = boost::container::static_vector<MyByte, MAX_PACKET_PAYLOAD_SIZE>;
using PacketBuffer = std::array<MyByte, MAX_PACKET_SIZE>;
using PacketMetadata = std::array<MyByte, PACKET_HEADER_SIZE>;

enum class PacketTypes : MyByte {
	Image = static_cast<MyByte>('I'),
	Invalid = static_cast<MyByte>('\0')
};

// Holds _metadata about a packet
class PacketHeader {

	friend class Packet;

public:

	static const Ushort SIZE_OFFSET = 1;

	PacketHeader();

protected:

	PacketMetadata _metadata;

	PacketHeader(const PacketHeader& header);
	PacketHeader(PacketHeader&& header);

public:

	PacketHeader(const PacketBuffer& packetBuffer);
	PacketHeader(const PacketPayload& payload, const PacketMetadata& metadata);
	const Uint32 Size() const;

};

struct ImagePacketHeader : public PacketHeader {
	static const Ushort POSITION_OFFSET = SIZE_OFFSET + 4;

	const Uint32 Position() const;
	const Uint32 Size() const;

	ImagePacketHeader(const PacketHeader& header) : PacketHeader(header) {}
	ImagePacketHeader(const Uint32 size, const Uint32 position);
};

// Packet of data that can be sent over a socket
class Packet {

public:

	static const bool InvalidImagePacket(const PacketBuffer& packetBuffer);

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

};

using PacketList = std::vector<Packet>;