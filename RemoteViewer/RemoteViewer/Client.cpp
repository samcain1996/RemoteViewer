#include "Client.h"

void Client::ProcessPacket(const Packet packet) {
    // Get packet group
    uint32 group = packet.Header().group;

    ThreadLock lock(m_mutex);  // Prevent other threads from accessing variables used in scope

    // Add packet to list of packets in its group
    PacketPrioQueue& packetGroupBucket = m_incompletePackets[group];
    packetGroupBucket.push(packet);

    // Add payload size of packet to total message size
    m_packetGroupMsgSizes[group] += packet.Header().size - PACKET_HEADER_SIZE;

    m_checkPackets = true;  // Allow PacketWatcher to check for complete messages
}

void Client::PacketWatcher() {
    while (true) {

        // Sleep while not allowed to check packets
        while (!m_checkPackets) { std::this_thread::yield(); }

        ThreadLock lock(m_mutex);  // Lock variables in scope to current thread

        m_checkPackets = false;  // Reset flag

        // Check each group for a full queue
        for (auto& [group, prioQ] : m_incompletePackets) {

            // Get first packet in group that has arrived so far
            const Packet& packet = prioQ.top();

            // When the top packet is the first in the sequence,
            // we can check that its payload is the same size as
            // the queue. This implies that all the packets are here
            // and the message can be assembled
            if (packet.Header().sequence == 0) {

                if (prioQ.size() == packet.Header().size) {
                    AssembleMessage(packet.Header().group);
                }
            }

        }

    }
}

Client::Client(const ushort port, const std::string& hostname) : NetAgent(port) {

	m_hostname = hostname;

	packetWatcherThr = std::thread(&Client::PacketWatcher, this);
}


bool Client::Connect(const std::string& serverPort) {

    // Find endpoint to connect to
    udp::resolver resolver(io_context);
    m_remoteEndpoint = *resolver.resolve(udp::v4(), m_hostname, serverPort).begin();

    // Handshake
    Byte handshakeBuf[sizeof HANDSHAKE_MESSAGE];
    m_socket.connect(m_remoteEndpoint);

    m_socket.send(boost::asio::buffer(HANDSHAKE_MESSAGE, sizeof HANDSHAKE_MESSAGE), NULL, m_errcode);
    m_socket.receive(boost::asio::buffer(handshakeBuf, sizeof HANDSHAKE_MESSAGE), NULL, m_errcode);

    if (memcmp(handshakeBuf, HANDSHAKE_MESSAGE, sizeof HANDSHAKE_MESSAGE)) { return false; }

    Receive();

}

std::vector<Byte> Client::AssembleMessage(const PacketGroup& group) {

    PacketPrioQueue& queue = m_incompletePackets[group];

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

    }
}

