#include "Client.h"

Client::Client(const Ushort port, const std::string& hostname) : NetAgent(port) {
    _hostname = hostname;
}

void Client::ProcessPacket(const Packet& packet) {
    // Get packet group
    Uint32 group = packet.Header().group;

    if (packet.Header().sequence == 0) {
        _packetGroups[group] = packet.Header().size - 1;
        return;
    }

    ThreadLock lock(_mutex);  // Prevent other threads from accessing variables used in scope

    // Add packet to list of packets in its group
    PacketPriorityQueue& packetGroupBucket = _incompletePackets[group];
    packetGroupBucket.push(packet);

    if (_packetGroups[group] == packetGroupBucket.size()) {
        writer->WriteMessage(group);
        _packetGroups.erase(group);
    }
}

bool Client::Connect(const std::string& serverPort) {

    // Find endpoint to connect to
    udp::resolver resolver(_io_context);
    _remoteEndpoint = *resolver.resolve(udp::v4(), _hostname, serverPort).begin();

    // Handshake
    Byte handshakeBuf[HANDSHAKE_SIZE];
    _socket.connect(_remoteEndpoint);

    _socket.send(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_SIZE), 0, _errcode);
    _socket.receive(boost::asio::buffer(handshakeBuf, HANDSHAKE_SIZE), 0, _errcode);

    if (std::memcmp(handshakeBuf, HANDSHAKE_MESSAGE, HANDSHAKE_SIZE)) { return false; }

    return true;
}

void Client::Receive() {
   
    PacketBuffer packetData;

    while (_keepAlive) {

        // Receive packet
        _socket.receive(boost::asio::buffer(packetData, packetData.max_size()), 0, _errcode);
        _socket.send(boost::asio::buffer(packetData, 1), 0, _errcode);

        // Copy buffer to dummy packet
        ProcessPacket(Packet(packetData));
    }

}

Client::~Client() {

}

