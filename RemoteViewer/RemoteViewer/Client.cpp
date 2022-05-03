#include "Client.h"

void Client::ProcessPacket(const Packet packet) {
    // Get packet group
    uint32 group = packet.Header().group;

    ThreadLock lock(m_mutex);  // Prevent other threads from accessing variables used in scope

    // Add packet to list of packets in its group
    PacketPrioQueue& packetGroupBucket = _incompletePackets[group];
    packetGroupBucket.push(packet);

    _checkPackets = true;  // Allow PacketWatcher to check for complete messages
}

void Client::PacketWatcher() {

    while (true) {

        // Sleep while not allowed to check packets
        while (!_checkPackets) { std::this_thread::yield(); }

        ThreadLock lock(m_mutex);  // Lock variables in scope to current thread

        if (stoken.stop_requested()) { return; }

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
                    ByteVec img = AssembleMessage(packet.Header().group);
                    std::ofstream out("received.bmp", std::ios_base::binary);
                    for (auto& x : img) {
                        out << x; 
                    }
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
    udp::resolver resolver(io_context);
    m_remoteEndpoint = *resolver.resolve(udp::v4(), _hostname, serverPort).begin();

    // Handshake
    Byte handshakeBuf[sizeof HANDSHAKE_MESSAGE];
    m_socket.connect(m_remoteEndpoint);

    m_socket.send(boost::asio::buffer(HANDSHAKE_MESSAGE, sizeof HANDSHAKE_MESSAGE), NULL, m_errcode);
    m_socket.receive(boost::asio::buffer(handshakeBuf, sizeof HANDSHAKE_MESSAGE), NULL, m_errcode);

    if (memcmp(handshakeBuf, HANDSHAKE_MESSAGE, sizeof HANDSHAKE_MESSAGE)) { return false; }

    Receive();

}

std::vector<Byte> Client::AssembleMessage(const PacketGroup& group) {

    PacketPrioQueue& queue = _incompletePackets[group];

    std::vector<Byte> fullMessage;

    queue.pop();

    for (size_t packetNo = 0; !queue.empty(); packetNo++) {
        const Packet& packet = queue.top();
        PacketPayload p(packet.Payload());

        std::copy(p.begin(), p.end(), std::back_inserter(fullMessage));

        queue.pop();
    }

    return fullMessage;
}

void Client::Receive() {
    bool keepAlive = true;
    while (keepAlive) {

        PacketBuffer packetData;

        // Receive packet
        m_socket.receive(boost::asio::buffer(packetData, packetData.max_size()), NULL, m_errcode);
        m_socket.send(boost::asio::buffer(packetData, sizeof Byte), NULL, m_errcode);

        // Copy buffer to dummy packet
        ProcessPacket(Packet(packetData));

        _packetWatcherThr.request_stop();
        _packetWatcherThr.join();

    }
}

