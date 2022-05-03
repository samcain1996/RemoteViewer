#include "Server.h"

Server::Server(const unsigned short listenPort) : NetAgent(listenPort),
    randomGenerator(rd())  {}

void Server::Listen() {

    Byte listenBuf[PACKET_HEADER_ELEMENT_SIZE];  // Buffer to hold handshake message

    m_socket.receive_from(boost::asio::buffer(listenBuf, sizeof listenBuf), m_remoteEndpoint, NULL, m_errcode);

    // If message is not the handshake message, start over
    if (memcmp(listenBuf, HANDSHAKE_MESSAGE, sizeof listenBuf)) { Listen(); }

    m_socket.send_to(boost::asio::buffer(HANDSHAKE_MESSAGE, sizeof HANDSHAKE_MESSAGE), m_remoteEndpoint, NULL, m_errcode);

    Serve();
}

void Server::Serve() {
    bool keepAlive = true;
    //while (keepAlive) {
        std::string message = "Hello, World!";
        ushort len = static_cast<ushort>(strlen(message.c_str()));
        /*std::string& nextMessage = messageQueue.GetNext();*/
        std::vector<Byte> nextMsgBytes{ 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' };

        Send(nextMsgBytes);
    //}
}

void Server::Send(std::vector<Byte>& bytes) {

    // Convert the message into a list of packets
    PacketList packets = ConvertToPackets(bytes);

    // Loop through all the packets and send them
    for (size_t packetNo = 0; packetNo < packets.size(); packetNo++) {
        Packet& packet = packets[packetNo];

        Byte dummyBuf[1];

        // Send packet and then wait for acknowledgment
        m_socket.send_to(boost::asio::buffer(packet.RawData(), MAX_PACKET_SIZE), m_remoteEndpoint, NULL, m_errcode);
        m_socket.receive(boost::asio::buffer(dummyBuf, 1), NULL, m_errcode);
    }

}

PacketList Server::ConvertToPackets(std::vector<Byte>& bytes)
{
    // Assign all packets that are part of this message to a group
    PacketHeader header;
    header.group = randomGenerator();

    // Calculate the number of packets that will
    // need to be send in order to send entire message
    uint32 numberOfPackets = bytes.size() / MAX_PACKET_PAYLOAD_SIZE + 2;

    PacketList packets;

    // Create the first packet, the first packet
    // has a sequence of 0 and its payload is
    // the number of total packets in the message
    header.size = numberOfPackets;
    header.sequence = 0;

    PacketPayload payload;
    packets.push_back(Packet(header, payload));  // Add header to list of packets to send

    // Break message down into packets
    for (ushort bytesRemaining = bytes.size(), iteration = 0; bytesRemaining > 0; iteration++) {

        ushort offset = iteration * MAX_PACKET_PAYLOAD_SIZE;                // Current offset in message for current packet

        // Payload will always be the maximum size, unless less room is needed
        ushort payloadSize = bytesRemaining < MAX_PACKET_PAYLOAD_SIZE ? bytesRemaining : MAX_PACKET_PAYLOAD_SIZE;
        ushort totalSize = payloadSize + PACKET_HEADER_SIZE;  // Size of the entire packet

        // Assemble packet
        header.size = totalSize;              // Packet length in bytes
       
        header.sequence = iteration + 1;        // Packet sequence in group
        std::copy((bytes.begin() + offset), (bytes.begin() + offset + payloadSize), payload.begin());

        packets.push_back(Packet(header, payload));

        bytesRemaining -= payloadSize;
    }

    return packets;
}
