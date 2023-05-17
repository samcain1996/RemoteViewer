#include "Networking/NetAgent.h"

NetAgent::NetAgent() {}

bool NetAgent::Connected() const {
    return connections.empty() ? false : std::all_of(connections.begin(), connections.end(), 
        [](const ConnectionPtr& pConnection) {
            return pConnection->connected;
        }
    ); 
};

bool NetAgent::IsDisconnectMsg(const PacketBuffer& buffer) const {
    return std::memcmp(buffer.data(), DISCONNECT_MESSAGE.data(), DISCONNECT_MESSAGE.size()) == 0;
}

bool NetAgent::portAvailable(unsigned short port) {

    io_context context;
    tcp::acceptor a(context);

    error_code ec;
    a.open(tcp::v4(), ec) || a.bind({ tcp::v4(), port }, ec);

    return ec != boost::asio::error::address_in_use;

}

void NetAgent::Handshake(ConnectionPtr& pConnection) {

    // Determine operating system
    const bool isMac     = std::memcmp(pConnection->buffer.data(), MAC_HANDSHAKE.data(), MAC_HANDSHAKE.size()) == 0;
    const bool isLinux   = std::memcmp(pConnection->buffer.data(), LIN_HANDSHAKE.data(), LIN_HANDSHAKE.size()) == 0;
    const bool isWindows = std::memcmp(pConnection->buffer.data(), WIN_HANDSHAKE.data(), WIN_HANDSHAKE.size()) == 0;

    if (isMac) { _connectedOS = OperatingSystem::MAC; }
    else if (isLinux) { _connectedOS = OperatingSystem::LINUX; }
    else if (isWindows) { _connectedOS = OperatingSystem::WINDOWS; }

    // Connected if one of the following operating systems was connected to
    pConnection->connected = ( isMac || isLinux || isWindows );

}

void NetAgent::Disconnect() {

    for_each(connections.begin(), connections.end(), [](auto& pConnection) {

        auto& pSocket = pConnection->pSocket;

        // Disconnect
        if (pSocket->is_open()) {
            pSocket->write_some(boost::asio::buffer(DISCONNECT_MESSAGE), pConnection->errorcode);
            pSocket->cancel();
        }

        pConnection->connected = false;

    });
}

PacketList NetAgent::ConvertToPackets(const PixelData& data, const PacketType& packetType)
{
    Uint32 group = randomGenerator();

    // Calculate the number of packets that will
    // need to be sent in order to send entire message
    Uint32 numberOfPackets = (Uint32)std::ceil(
        ((double)data.size() / MAX_PACKET_PAYLOAD_SIZE));

    PacketList packets(numberOfPackets);

    // Break message down into packets
    for (size_t bytesRemaining = data.size(), iteration = 0; bytesRemaining > 0; iteration++) {

        size_t offset = iteration * MAX_PACKET_PAYLOAD_SIZE; // Current offset in message for current packet

        // Payload will always be the maximum size, unless less room is needed
        Ushort payloadSize = bytesRemaining < MAX_PACKET_PAYLOAD_SIZE ? bytesRemaining : MAX_PACKET_PAYLOAD_SIZE;
        Ushort totalSize = payloadSize + PACKET_HEADER_SIZE;  // Size of the entire packet

        // Assemble packet
        PacketPayload payload(payloadSize);
        std::copy(data.begin() + offset, data.begin() + offset + payloadSize, payload.begin());

        ImagePacketHeader header(group, totalSize, iteration);
        packets[iteration] = Packet(header, payload);

        bytesRemaining -= payloadSize;
    }

    return packets;
}