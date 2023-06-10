#include "Networking/Packet.h"
#include "QuickShot/Capture.h"

PacketPtr Packet::VerifyPacket(const PacketBuffer& packet) {
	TempHeader header = TempHeader{ packet }.subspan(0, PACKET_HEADER_SIZE);

	bool match = PacketHeader::GetType(header) != PacketType::Invalid;
	match &= PacketHeader::Size(header) <= MAX_PACKET_SIZE;
	if (match) {
		return make_shared<Packet>(packet);
	}
	return nullptr;
}

bool Packet::VerifyPacket(const Packet& packet) {

	TempHeader header = TempHeader{ packet.RawData() }.subspan(0, PACKET_HEADER_SIZE);

	bool match = PacketHeader::GetType(header) != PacketType::Invalid;
	
	if (!(match &= PacketHeader::Size(header) <= MAX_PACKET_SIZE)) { return false; }

	return packet.Payload().size() == PacketHeader::Size(header) - PACKET_HEADER_SIZE;
}

Packet::Packet() : _header(PacketHeader()), _payload() {}

Packet::Packet(const Packet& other) : _header(other.Header()) {

	_payload = other._payload;
}

Packet::Packet(Packet&& other) noexcept : _header(other.Header()) {
	_payload = move(other._payload);
}

Packet::Packet(const PacketBuffer& packetData) {
	std::copy(packetData.begin(), packetData.begin() + PACKET_HEADER_SIZE, _header._metadata.begin());

	auto end = packetData.begin() + _header.Size();
	std::copy(packetData.begin() + PACKET_HEADER_SIZE, end, std::back_inserter(_payload));
}

Packet::Packet(const PacketHeader& header, const PacketPayload& payload) {

	_header._metadata = header._metadata;

	// Copy payload
	_payload = payload;
}

Packet& Packet::operator=(const Packet& other) {
	_header._metadata = other._header._metadata;
	_payload = other._payload;

	return *this;
}

Packet& Packet::operator=(Packet&& other) noexcept {
	_header._metadata = move(other._header._metadata);
	_payload = move(other._payload);

	return *this;
}

const PacketBuffer Packet::RawData() const {
	std::array<MyByte, MAX_PACKET_SIZE> data;  // Entire packet as contiguous array

	// Append the payload
	std::copy(_header._metadata.begin(), _header._metadata.end(), data.begin());
	std::copy(_payload.begin(), _payload.end(), data.begin() + PACKET_HEADER_SIZE);

	return data;
}

const PacketHeader Packet::Header() const {
	return _header;
}

const PacketPayload Packet::Payload() const {
	return _payload;
}


Uint32 ImagePacketHeader::Position() const {
	return DecodeAsByte(&_metadata.data()[POSITION_OFFSET]);
}

Uint32 PacketHeader::Size() const {
	return PacketHeader::Size(_metadata);
}

Uint32 PacketHeader::Group() const {
	return PacketHeader::Group(_metadata);
}

Uint32 PacketHeader::Group(const TempHeader& header) {
	return DecodeAsByte(&header.data()[GROUP_OFFSET]);
}

Uint32 PacketHeader::Size(const TempHeader& header) {
	return DecodeAsByte(&header.data()[SIZE_OFFSET]);
}


PacketType PacketHeader::Type() const {
	return PacketHeader::GetType(_metadata);
}

PacketHeader::PacketHeader(const PacketBuffer& packetBuffer) {
	std::copy(packetBuffer.begin(), packetBuffer.begin() + _metadata.size(), _metadata.begin());
}

PacketHeader::PacketHeader() {
	_metadata.fill('\0');
}

PacketHeader::PacketHeader(const PacketHeader& header) {
	_metadata = header._metadata;
}

PacketHeader::PacketHeader(PacketHeader&& header) noexcept {
	_metadata = move(header._metadata);
}

PacketHeader::PacketHeader(const PacketPayload& payload, const PacketMetadata& metadata) {
	_metadata = metadata;
	EncodeAsByte(&_metadata.data()[SIZE_OFFSET], payload.size());
}

ImagePacketHeader::ImagePacketHeader(const Uint32 group, const Uint32 size, const Uint32 position) : PacketHeader() {
	_metadata[0] = static_cast<MyByte>(PacketType::Image);
	EncodeAsByte(&_metadata.data()[GROUP_OFFSET], group);
	EncodeAsByte(&_metadata.data()[SIZE_OFFSET], size);
	EncodeAsByte(&_metadata.data()[POSITION_OFFSET], position);
}

MousePacketHeader::MousePacketHeader(const Uint32 group, const Uint32 size) : PacketHeader() {
	_metadata[0] = static_cast<MyByte>(PacketType::Mouse);
	EncodeAsByte(&_metadata.data()[GROUP_OFFSET], group);
	EncodeAsByte(&_metadata.data()[SIZE_OFFSET], size);
}
