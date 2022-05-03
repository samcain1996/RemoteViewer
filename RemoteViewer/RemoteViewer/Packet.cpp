#include "Packet.h"

bool operator<(const Packet& p1, const Packet& p2) {
	return p1._header.sequence > p2._header.sequence;
}

Packet::Packet(const Packet& other) {
	_header = other._header;
	_payload = other._payload;
}

Packet::Packet(Packet&& other) noexcept {
	_header = std::move(other._header);
	_payload = std::move(other._payload);
}

Packet::Packet(PacketBuffer packetData) {
	ByteEncodedUint32 encoded;

	std::memmove(encoded, packetData.data(), sizeof ByteEncodedUint32);
	_header.size = decode256(encoded);

	std::memmove(encoded, &(packetData.data())[PACKET_GROUP_OFFSET], sizeof ByteEncodedUint32);
	_header.group = decode256(encoded);

	std::memmove(encoded, &(packetData.data())[PACKET_SEQUENCE_OFFSET], sizeof ByteEncodedUint32);
	_header.sequence = decode256(encoded);

	std::memmove(_payload.data(), &packetData[PACKET_PAYLOAD_OFFSET], _payload.size());
}

Packet::Packet(const PacketHeader& header, const PacketPayload& payload) {

	_header.size	 = header.size;
	_header.group    = header.group;
	_header.sequence = header.sequence;

	std::copy(payload.begin(), payload.end(), _payload.begin());
}

Packet& Packet::operator=(const Packet& other) {
	_header = other._header;
	_payload = other._payload;
	return *this;
}

const PacketBuffer Packet::RawData() const {
	std::array<Byte, MAX_PACKET_SIZE> data;
	std::array<Byte, PACKET_HEADER_ELEMENT_SIZE> encoded;

	encode256(encoded.data(), _header.size);
	std::copy(encoded.begin(), encoded.end(), data.begin());

	encode256(encoded.data(), _header.group);
	std::copy(encoded.begin(), encoded.end(), (data.begin() + PACKET_GROUP_OFFSET));

	encode256(encoded.data(), _header.sequence);
	std::copy(encoded.begin(), encoded.end(), (data.begin() + PACKET_SEQUENCE_OFFSET));

	std::copy(_payload.begin(), _payload.end(), (data.begin() + PACKET_PAYLOAD_OFFSET));

	return data;
}

const PacketHeader Packet::Header() const {
	return _header;
}

const PacketPayload Packet::Payload() const {
	return _payload;
}
