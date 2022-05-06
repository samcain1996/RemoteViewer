#include "Client.h"

void Client::ProcessPacket(const Packet packet) {
    // Get packet group
    uint32 group = packet.Header().group;

    ThreadLock lock(_mutex);  // Prevent other threads from accessing variables used in scope

    // Add packet to list of packets in its group
    PacketPrioQueue& packetGroupBucket = _incompletePackets[group];
    packetGroupBucket.push(packet);

    _checkPackets = true;  // Allow PacketWatcher to check for complete messages
}

void Client::PacketWatcher() {

    while (true) {

        // Sleep while not allowed to check packets
        while (!_checkPackets) { std::this_thread::yield(); }

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
                    ByteArray arr;

                    size_t size = AssembleMessage(packet.Header().group, arr);

                    _incompletePackets.erase(packet.Header().group);

                    std::ofstream out("received.bmp", std::ios_base::binary);
                    out.write((char*)arr, size);
                    out.close();
                }
            }

        }

    }
}

Client::Client(const ushort port, const std::string& hostname) : NetAgent(port) {

	_hostname = hostname;

	_packetWatcherThr = std::thread(&Client::PacketWatcher, this);
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

size_t Client::AssembleMessage(const PacketGroup& group, ByteArray& arr) {

    PacketPrioQueue& queue = _incompletePackets[group];

    //ByteVec fullMessage;

    queue.pop();  // Do not count the 'head' packet

    size_t size = 0;
    arr = new Byte[queue.size() * MAX_PACKET_PAYLOAD_SIZE];

    for (size_t packetNo = 0; !queue.empty(); packetNo++) {

        // Retrieve payload from top packet
        const Packet& packet = queue.top();

        // Append payload to vector
        size += packet.Header().size - PACKET_HEADER_SIZE;
        std::memcpy(&arr[(packet.Header().sequence - 1) * MAX_PACKET_PAYLOAD_SIZE], packet.Payload().data(), packet.Header().size - PACKET_HEADER_SIZE);

        queue.pop();  // Remove packet from queue
    }

    return size;
}

void Client::Receive() {
    bool keepAlive = true;
    while (keepAlive) {

        PacketBuffer packetData;

        // Receive packet
        _socket.receive(boost::asio::buffer(packetData, packetData.max_size()), 0, _errcode);
        _socket.send(boost::asio::buffer(packetData, 1), 0, _errcode);

        // Copy buffer to dummy packet
        ProcessPacket(Packet(packetData));

    }
}

