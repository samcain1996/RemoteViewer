#include "Networking/Packet.h"

const bool Packet::InvalidImagePacket(const PacketBuffer& packetBuffer) {
	
	int expectedPackets = ceil(ScreenCapture::CalculateBMPFileSize() - BMP_HEADER_SIZE
		/ (double)MAX_PACKET_PAYLOAD_SIZE);

	const PacketHeader header(packetBuffer);
	const ImagePacketHeader imageheader(header);

	return imageheader.Size() > MAX_PACKET_SIZE || imageheader.Position() > expectedPackets;
	
}

const Uint32 Packet::DecodeAsByte(const Byte encodedNumber[4])
{

	Byte temp[4];
	std::memcpy(temp, encodedNumber, 4);

	return ((Uint32)temp[0] + ((Uint32)temp[1] << ONE_BYTE) +
		((Uint32)temp[2] << TWO_BYTES) + ((Uint32)temp[3] << THREE_BYTES));
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

Packet::Packet(const PacketHeader& header, const PacketPayload& payload) : 
	_header(header) {

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


const Uint32 ImagePacketHeader::Position() const
{
	return Packet::DecodeAsByte(&_metadata.data()[POSITION_OFFSET]);
}

const Uint32 ImagePacketHeader::Size() const
{
	return PacketHeader::Size();
}

const Uint32 PacketHeader::Size() const
{
	return Packet::DecodeAsByte(&_metadata.data()[SIZE_OFFSET]);
}

PacketHeader::PacketHeader(const PacketBuffer& packetBuffer)
{
	std::copy(packetBuffer.begin(), packetBuffer.begin() + _metadata.size(), _metadata.begin());
}

PacketHeader::PacketHeader(const PacketHeader& header) : _metadata(header._metadata)
{
}

PacketHeader::PacketHeader(const PacketPayload& payload, const PacketMetadata& metadata) :
	_metadata(metadata)
{
	EncodeAsByte(&_metadata.data()[SIZE_OFFSET], payload.size());
}
