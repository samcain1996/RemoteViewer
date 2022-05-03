#pragma once
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <array>
#include <memory>
#include "Types.h"

#define MAX_PACKET_SIZE					4096

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

struct PacketHeader {
	uint32 size;	  // Packet payload + packet header size
	uint32 group;     // Group the packet belongs to
	uint32 sequence;  // Sequence in the group the packet is
};

class Packet {
	bool friend operator<(const Packet&, const Packet&);
private:
	PacketHeader  _header;
	PacketPayload _payload;
public:
	Packet() = delete;

	Packet(const Packet&);
	Packet(Packet&&) noexcept;
	
	Packet(PacketBuffer);
	Packet(const PacketHeader&, const PacketPayload&);

	Packet& operator=(const Packet&);

	const PacketBuffer	    RawData() const;
	const PacketHeader	    Header()  const;
	const PacketPayload		Payload() const;
};

using PacketList		= std::vector<Packet>;
using PacketPrioQueue	= std::priority_queue<Packet, std::vector<Packet>,  std::less<Packet>>;

template <>
class std::hash<PacketPrioQueue> {
public:
	size_t operator()(const PacketPrioQueue& queue) const {
		return queue.top().Header().size;
	}
};