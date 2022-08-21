#pragma once
#include <unordered_map>
#include <boost/container/static_vector.hpp>
#include "Types.h"

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

using PacketGroup   = Uint32;
using PacketPayload = boost::container::static_vector<Byte, MAX_PACKET_PAYLOAD_SIZE>;
using PacketBuffer  = std::array<Byte, MAX_PACKET_SIZE>;

// Holds metadata about a packet
struct PacketHeader {
	Uint32 size;	  // Packet payload + packet header size
	Uint32 group;     // Group the packet belongs to
	Uint32 sequence;  // Sequence in the group the packet is
};

// Packet of data that can be sent over a socket
class Packet {
	bool friend operator<(const Packet&, const Packet&);
private:
	PacketHeader  _header;		// Header containing packet metadata
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

	const PacketBuffer	    RawData() const;  // Metadata and payload in a contiguous array
	const PacketHeader	    Header()  const;  // Metadata
	const PacketPayload		Payload() const;  // Payload

};

using PacketList			      = std::vector<Packet>;
using PacketPriorityQueue		  = std::priority_queue<Packet, PacketList, std::less<Packet> >;

using PacketGroupPriorityQueueMap = std::unordered_map<Uint32, PacketPriorityQueue>;
using PacketGroupMap			  = std::unordered_map<Uint32, Uint32>;

// Hash function for Packet priority queue.
// Ideally, all packets with the same group will have
// the same message. So hash can just be the group # for now.
template <>
class std::hash<PacketPriorityQueue> {
public:
	size_t operator()(const PacketPriorityQueue& queue) const {
		return queue.top().Header().group;
	}
};