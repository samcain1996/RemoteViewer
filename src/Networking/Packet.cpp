#include "Networking/Packet.h"

const bool Packet::InvalidImagePacket(const PacketBuffer& packetBuffer) {
	
	int expectedPackets = ceil(ScreenCapture::CalculateBMPFileSize()
		/ (double)MAX_PACKET_PAYLOAD_SIZE);

	const ImagePacketHeader imageheader(packetBuffer);

	return imageheader.Size() > MAX_PACKET_SIZE || imageheader.Position() > expectedPackets;
	
}

Packet::Packet(const Packet& other) : _header(other.Header()) {

	_payload = other._payload;
}

Packet::Packet(Packet&& other) noexcept : _header(other.Header()) {
	_payload = std::move(other._payload);
}

Packet::Packet(const PacketBuffer& packetData) :
	_header(packetData) {
	
	// Retrieve payload
	std::copy(packetData.begin() + PACKET_PAYLOAD_OFFSET,
		packetData.begin() + _header.Size(), std::back_inserter(_payload));
}

Packet::Packet(const PacketHeader& header, const PacketPayload& payload)  {

	_header._metadata = header._metadata;

	// Copy payload
	_payload = payload;
}

Packet& Packet::operator=(const Packet& other) {
	_header._metadata  = other._header._metadata;
	_payload = other._payload;

	return *this;
}

Packet& Packet::operator=(Packet&& other) noexcept {
	_header._metadata = std::move(other._header._metadata);
	_payload = std::move(other._payload);

	return *this;
}

const PacketBuffer Packet::RawData() const {
	std::array<Byte, MAX_PACKET_SIZE> data;  // Entire packet as contiguous array

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


const Uint32 ImagePacketHeader::Position() const {
	return DecodeAsByte(&_metadata.data()[POSITION_OFFSET]);
}

const Uint32 ImagePacketHeader::Size() const {
	return PacketHeader::Size();
}

const Uint32 PacketHeader::Size() const {
	return DecodeAsByte(&_metadata.data()[SIZE_OFFSET]);
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

ImagePacketHeader::ImagePacketHeader(const Uint32 size, const Uint32 position) : PacketHeader() {
		_metadata[0] = static_cast<Byte>(PacketTypes::Image);
		EncodeAsByte(&_metadata.data()[SIZE_OFFSET], size);
		EncodeAsByte(&_metadata.data()[POSITION_OFFSET], position);
}
