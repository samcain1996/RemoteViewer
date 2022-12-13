#include "Networking/Packet.h"
#include "QuickShot/Capture.h"

Packet::Packet(const Packet& other) : _header(other.Header()) {

	_payload = other._payload;
}

Packet::Packet(Packet&& other) noexcept : _header(other.Header()) {
	_payload = std::move(other._payload);
}

Packet::Packet(const PacketBuffer& packetData) {
	std::memcpy(&_header._metadata[0], packetData.data(), PACKET_HEADER_SIZE);
	std::copy(packetData.begin() + PACKET_HEADER_SIZE, packetData.end(), std::back_inserter(_payload));
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
	_header._metadata = std::move(other._header._metadata);
	_payload = std::move(other._payload);

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

//Uint32 ImagePacketHeader::Size() const {
//	return PacketHeader::Size();
//}
//
//Uint32 ImagePacketHeader::Group() const {
//	return PacketHeader::Group();
//}

Uint32 PacketHeader::Size() const {
	return DecodeAsByte(&_metadata.data()[SIZE_OFFSET]);
}

Uint32 PacketHeader::Group() const {
	return DecodeAsByte(&_metadata.data()[GROUP_OFFSET]);
}

PacketType PacketHeader::Type() const {
	return GetType(*this);
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

PacketHeader::PacketHeader(PacketHeader&& header) {
	_metadata = std::move(header._metadata);
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
