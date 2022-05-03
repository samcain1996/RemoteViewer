#pragma once
#include <vector>
#include <queue>
#include <array>
#include "Types.h"

// Defines related to packets

#define MAX_PACKET_SIZE					20

#define PACKET_HEADER_COUNT				3
#define PACKET_HEADER_ELEMENT_SIZE		4
#define PACKET_HEADER_SIZE				(PACKET_HEADER_ELEMENT_SIZE * PACKET_HEADER_COUNT)

#define MAX_PACKET_PAYLOAD_SIZE			(MAX_PACKET_SIZE - PACKET_HEADER_SIZE)

#define PACKET_ENCODED_OFFSET			0
#define PACKET_GROUP_OFFSET				(PACKET_HEADER_ELEMENT_SIZE)
#define PACKET_SEQUENCE_OFFSET			(PACKET_HEADER_ELEMENT_SIZE * 2)
#define PACKET_PAYLOAD_OFFSET			PACKET_HEADER_SIZE

using PacketGroup   = uint32;
using PacketPayload = std::array<Byte, MAX_PACKET_PAYLOAD_SIZE>;
using PacketBuffer  = std::array<Byte, MAX_PACKET_SIZE>;

// Holds metadata about a packet
struct PacketHeader {
	uint32 size;	  // Packet payload + packet header size
	uint32 group;     // Group the packet belongs to
	uint32 sequence;  // Sequence in the group the packet is
};

// Packet of data that can be sent over a socket
class Packet {
	bool friend operator<(const Packet&, const Packet&);
private:
	PacketHeader  _header;		// Header containing packet metadata
	PacketPayload _payload;		// Packet data
public:
	// Packets should only be constructed from:
	//	1. Another packet
	//  2. A buffer representing the header and payload
	//  3. A header and payload
	Packet() = delete;

	Packet(const Packet&);
	Packet(Packet&&) noexcept;
	
	Packet(PacketBuffer);
	Packet(const PacketHeader&, const PacketPayload&);

	Packet& operator=(const Packet&);

	const PacketBuffer	    RawData() const;  // Metadata and payload in a contiguous array
	const PacketHeader	    Header()  const;  // Metadata
	const PacketPayload		Payload() const;  // Payload
};

using PacketList		= std::vector<Packet>;
using PacketPrioQueue	= std::priority_queue<Packet, std::vector<Packet>, std::less<Packet> >;

// Hash function for Packet priority queue.
// Ideally, all packets with the same group will have
// the same message. So hash can just be the group # for now.
template <>
class std::hash<PacketPrioQueue> {
public:
	size_t operator()(const PacketPrioQueue& queue) const {
		return queue.top().Header().group;
	}
};