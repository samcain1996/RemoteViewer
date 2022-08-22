#include "Networking/NetAgent.h"

std::random_device NetAgent::rd{};

std::mt19937 NetAgent::randomGenerator(rd());

NetAgent::NetAgent(const std::chrono::seconds& timeout) : _socket(_io_context), _timeout(timeout) {}

NetAgent::~NetAgent() { }

void NetAgent::Receive() {}

const bool NetAgent::IsDisconnectMsg() const {

	return std::memcmp(_tmpBuffer.data(), DISCONNECT_MESSAGE.data(), DISCONNECT_MESSAGE.size()) == 0;

}

void NetAgent::Disconnect() {
    _connected = false; 
    _socket.write_some(boost::asio::buffer(DISCONNECT_MESSAGE), _errcode);
}

PacketList NetAgent::ConvertToPackets(const ByteVec& data)
{
    PacketList packets;  // List to hold all packets needed to create message

    // Assign all packets that are part of this message to a group
    PacketHeader header{};
    header.group = randomGenerator();

    // Calculate the number of packets that will
    // need to be send in order to send entire message
    Uint32 numberOfPackets = (Uint32)std::ceil(
        ((float)data.size() / MAX_PACKET_PAYLOAD_SIZE));

    packets.reserve(numberOfPackets);

    // Break message down into packets
    for (size_t bytesRemaining = data.size(), iteration = 0; bytesRemaining > 0; iteration++) {

        size_t offset = iteration * MAX_PACKET_PAYLOAD_SIZE; // Current offset in message for current packet

        // Payload will always be the maximum size, unless less room is needed
        Ushort payloadSize = bytesRemaining < MAX_PACKET_PAYLOAD_SIZE ? bytesRemaining : MAX_PACKET_PAYLOAD_SIZE;
        Ushort totalSize = payloadSize + PACKET_HEADER_SIZE;  // Size of the entire packet

        // Assemble packet
        PacketPayload payload(payloadSize); // Ensure payload is empty

        header.size = totalSize;     // Packet length in bytes
        header.sequence = iteration; // Packet sequence in group

        std::copy(data.begin() + offset, data.begin() + offset + payloadSize, payload.begin());

        packets.push_back(Packet(header, payload));

        bytesRemaining -= payloadSize;
    }

    return packets;
}

const bool NetAgent::Connected() const { return _connected; }