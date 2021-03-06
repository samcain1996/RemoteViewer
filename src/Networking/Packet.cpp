#include "Networking/Packet.h"

bool operator<(const Packet& p1, const Packet& p2) {
	// A packet that has a larger sequence number 
	// has a LOWER priority than one with a lower number;
	if (p1._header.group == p2._header.group) { 
		return p1._header.sequence > p2._header.sequence;
	}

	return p1._header.group > p2._header.group;
}

Packet::Packet(const Packet& other) {
	_header  = other._header;
	_payload = other._payload;
}

Packet::Packet(Packet&& other) noexcept {
	_header  = std::move(other._header);
	_payload = std::move(other._payload);
}

Packet::Packet(PacketBuffer& packetData) {
	
	ByteEncodedUint32 encoded;  // Temp variable to store encoded Uint32 values

	// Retrieve encoded size
	std::memcpy(encoded, packetData.data(), sizeof encoded);
	_header.size = decode256(encoded);

	// Retrieve encoded group
	std::memcpy(encoded, &(packetData.data())[PACKET_GROUP_OFFSET], sizeof encoded);
	_header.group = decode256(encoded);

	// Retrieve encoded sequence
	std::memcpy(encoded, &(packetData.data())[PACKET_SEQUENCE_OFFSET], sizeof encoded);
	_header.sequence = decode256(encoded);

	// Retrieve payload
	std::memcpy(_payload.data(), &packetData[PACKET_PAYLOAD_OFFSET], _payload.size());
}

Packet::Packet(const PacketHeader& header, const PacketPayload& payload) {

	// Set header vars
	_header.size	 = header.size;
	_header.group    = header.group;
	_header.sequence = header.sequence;

	// Copy payload
	std::copy(payload.begin(), payload.end(), _payload.begin());
}

Packet& Packet::operator=(const Packet& other) {
	_header  = other._header;
	_payload = other._payload;

	return *this;
}

Packet& Packet::operator=(Packet&& other) noexcept {
	_header = std::move(other._header);
	_payload = std::move(other._payload);

	return *this;
}

const PacketBuffer Packet::RawData() const {
	std::array<Byte, MAX_PACKET_SIZE> data;  // Entire packet as contiguous array
	std::array<Byte, PACKET_HEADER_ELEMENT_SIZE> encoded;  // Encoded header vars

	// Encode size into data
	encode256(encoded.data(), _header.size);
	std::copy(encoded.begin(), encoded.end(), data.begin());

	// Encode group into data
	encode256(encoded.data(), _header.group);
	std::copy(encoded.begin(), encoded.end(), (data.begin() + PACKET_GROUP_OFFSET));

	// Encode sequence into data
	encode256(encoded.data(), _header.sequence);
	std::copy(encoded.begin(), encoded.end(), (data.begin() + PACKET_SEQUENCE_OFFSET));

	// Append the payload
	std::copy(_payload.begin(), _payload.end(), (data.begin() + PACKET_PAYLOAD_OFFSET));

	return data;
}

const PacketHeader Packet::Header() const {
	return _header;
}

const PacketPayload Packet::Payload() const {
	return _payload;
}
