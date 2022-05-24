#include "Client.h"

Client::Client(const Ushort port, const std::string& hostname) : NetAgent(port) {
    _hostname = hostname;
    
    _packetWatcherThr = std::thread(&Client::PacketWatcher, this);
    _windowThr = std::thread([&](){ 
        _window = new RenderWindow(_hostname, &_incompletePackets, _msgWriter, &_keepAlive, &_mutex);
        _window->Run(); 
        });


}

void Client::ProcessPacket(const Packet packet) {
    // Get packet group
    Uint32 group = packet.Header().group;

    ThreadLock lock(_mutex);  // Prevent other threads from accessing variables used in scope

    // Add packet to list of packets in its group
    PacketPriorityQueue& packetGroupBucket = _incompletePackets[group];
    packetGroupBucket.push(packet);

    _checkPackets = true;  // Allow PacketWatcher to check for complete messages
}

void Client::PacketWatcher() {

    while (_keepAlive) {

        // Sleep while not allowed to check packets
        while (!_checkPackets) { if (!_keepAlive) return; std::this_thread::yield(); }

        ThreadLock lock(_mutex);  // Lock variables in scope to current thread

        _checkPackets = false;  // Reset flag

        // Check each group for a full queue
        for (auto& [group, prioQ] : _incompletePackets) {

            // Get first packet in group that has arrived so far
            const Packet& packet = prioQ.top();

            // When the top packet is the first in the sequence,
            // we can check that its payload is the same size as
            // the queue. This implies that all the packets are here
            // and the message can be assembled
            if (packet.Header().sequence == 0) {

                if (prioQ.size() == packet.Header().size) {
                    prioQ.pop();  // Remove 'Header' packet

                    _msgWriter.WriteMessage(packet.Header().group);
                }
            }

        }

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

    Receive();

    return true;
}

void Client::Receive() {
    int i = 0;
    
    PacketBuffer packetData;

    while (_keepAlive) {

        // Receive packet
        _socket.receive(boost::asio::buffer(packetData, packetData.max_size()), 0, _errcode);
        _socket.send(boost::asio::buffer(packetData, 1), 0, _errcode);

        if (memcmp(packetData.data(), DISCONNECT_MESSAGE, DISCONNECT_SIZE) == 0) { 
            _keepAlive = false;

        }

        // Copy buffer to dummy packet
        ProcessPacket(Packet(packetData));
    }

}

Client::~Client() {

    _windowThr.join();
    _packetWatcherThr.join();

}

