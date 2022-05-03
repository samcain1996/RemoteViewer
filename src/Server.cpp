#include "Server.h"

Server::Server(const unsigned short listenPort) : NetAgent(listenPort),
    randomGenerator(rd())  {}

void Server::Listen() {
    Byte connectionBuffer[PACKET_HEADER_ELEMENT_SIZE];  // Buffer to hold handshake message
    
    // Loop until the handshake message is received
    do {
        _socket.receive_from(boost::asio::buffer(connectionBuffer, sizeof connectionBuffer), _remoteEndpoint, 0, _errcode);
    } while(std::memcmp(connectionBuffer, HANDSHAKE_MESSAGE, HANDSHAKE_SIZE));

    // Send handshake back
    _socket.send_to(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_SIZE), _remoteEndpoint, 0, _errcode);

    Serve();
}

void Server::Serve() {
    bool keepAlive = false;

    // Loop indefinitely
    do {
        std::string test = "Hello, my name is Sam Cain";
        ByteVec message(test.begin(), test.end());

        Send(message);
    } while(keepAlive);
}

void Server::Send(ByteVec& bytes) {

    // Convert the message into a list of packets
    PacketList packets = ConvertToPackets(bytes);

    // Loop through all the packets and send them
    for (size_t packetNo = 0; packetNo < packets.size(); packetNo++) {
        Packet& packet = packets[packetNo];

        Byte dummyBuf[1];

        // Send packet and then wait for acknowledgment
        _socket.send_to(boost::asio::buffer(packet.RawData(), MAX_PACKET_SIZE), _remoteEndpoint, 0, _errcode);
        _socket.receive(boost::asio::buffer(dummyBuf, sizeof dummyBuf), 0, _errcode);
    }

}

PacketList Server::ConvertToPackets(ByteVec& message)
{
    PacketList packets;  // List to hold all packets needed to create message

    // Assign all packets that are part of this message to a group
    PacketHeader header;
    header.group = randomGenerator();

    // Calculate the number of packets that will
    // need to be send in order to send entire message
    uint32 numberOfPackets = std::ceil(
        ((float)message.size() / MAX_PACKET_PAYLOAD_SIZE)) + 1;

    // Create the first packet, the first packet
    // has a sequence of 0 and its size is the
    // number of packets in the entire message
    header.size = numberOfPackets;
    header.sequence = 0;

    // First packet gets dummy payload
    packets.push_back(Packet(header, PacketPayload()));  // Add header to list of packets to send

    // Break message down into packets
    for (size_t bytesRemaining = message.size(), iteration = 0; bytesRemaining > 0; iteration++) {

        size_t offset = iteration * MAX_PACKET_PAYLOAD_SIZE; // Current offset in message for current packet

        // Payload will always be the maximum size, unless less room is needed
        ushort payloadSize = bytesRemaining < MAX_PACKET_PAYLOAD_SIZE ? bytesRemaining : MAX_PACKET_PAYLOAD_SIZE;
        ushort totalSize = payloadSize + PACKET_HEADER_SIZE;  // Size of the entire packet

        // Assemble packet
        PacketPayload payload = PacketPayload();

        header.size     = totalSize;     // Packet length in bytes
        header.sequence = iteration + 1; // Packet sequence in group
        std::copy((message.begin() + offset), (message.begin() + offset + payloadSize), payload.begin());

        packets.push_back(Packet(header, payload));

        bytesRemaining -= payloadSize;
    }

    return packets;
}