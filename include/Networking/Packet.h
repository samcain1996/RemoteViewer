#pragma once

#include <unordered_map>
#include <boost/container/static_vector.hpp>
#include "Types.h"
#include "QuickShot/Capture.h"

// Defines related to packets

constexpr const Uint32 MAX_PACKET_SIZE = 4096;

constexpr const Uint32 PACKET_HEADER_COUNT = 3;
constexpr const Uint32 PACKET_HEADER_ELEMENT_SIZE = 4;
constexpr const Uint32 PACKET_HEADER_SIZE = (PACKET_HEADER_ELEMENT_SIZE * PACKET_HEADER_COUNT + 1);

constexpr const Uint32 MAX_PACKET_PAYLOAD_SIZE = (MAX_PACKET_SIZE - PACKET_HEADER_SIZE);

constexpr const Uint32 PACKET_TYPE_OFFSET = 0;
constexpr const Uint32 PACKET_GROUP_OFFSET = (PACKET_HEADER_ELEMENT_SIZE);
constexpr const Uint32 PACKET_SEQUENCE_OFFSET = (PACKET_HEADER_ELEMENT_SIZE * 2);
constexpr const Uint32 PACKET_PAYLOAD_OFFSET = PACKET_HEADER_SIZE;

using PacketGroup = Uint32;
using PacketPayload = boost::container::static_vector<MyByte, MAX_PACKET_PAYLOAD_SIZE>;
using PacketBuffer = std::array<MyByte, MAX_PACKET_SIZE>;
using PacketMetadata = std::array<MyByte, PACKET_HEADER_SIZE>;

using TempHeader = std::span<const MyByte>;
//using ConstTempHeader = std::span<const MyByte>;

enum class PacketType : MyByte {
	Image = static_cast<MyByte>('I'),
	Invalid = static_cast<MyByte>('\0')
};

// Holds _metadata about a packet
class PacketHeader {

	friend class Packet;

public:

	static const Ushort GROUP_OFFSET = 1;
	static const Ushort SIZE_OFFSET = GROUP_OFFSET + sizeof(Uint32);
	static inline PacketType GetType(const PacketHeader& header) {
		switch (header._metadata.data()[0]) {

			case 'I': return PacketType::Image;
			default:  return PacketType::Invalid;

		}
	}
	static inline PacketType GetType(TempHeader header) {
		switch (header.data()[0]) {

		case 'I': return PacketType::Image;
		default:  return PacketType::Invalid;

		}
	}
	PacketHeader();

protected:

	PacketMetadata _metadata;

	PacketHeader(const PacketHeader& header);
	PacketHeader(PacketHeader&& header) noexcept;

public:

	PacketHeader(const PacketBuffer& packetBuffer);
	PacketHeader(const PacketPayload& payload, const PacketMetadata& metadata);
	Uint32 Size() const;
	Uint32 Group() const;
	static Uint32 Size(const TempHeader&);
	static Uint32 Group(const TempHeader&);

	PacketType Type() const;
	explicit operator std::string() const { return std::string((char*)_metadata.data()); }

};

struct ImagePacketHeader : public PacketHeader {
	static const Ushort POSITION_OFFSET = SIZE_OFFSET + sizeof(Uint32);

	Uint32 Position() const;

	ImagePacketHeader(const PacketHeader& header) : PacketHeader(header) {}
	ImagePacketHeader(const Uint32 group, const Uint32 size, const Uint32 position);
};


// Packet of data that can be sent over a socket
class Packet {

	using PacketPtr = std::shared_ptr<Packet>;

public:

	static PacketPtr VerifyPacket(const PacketBuffer& packet);
	static bool VerifyPacket(const Packet& packet);

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
	Packet();

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
using PacketPtr = std::shared_ptr<Packet>;