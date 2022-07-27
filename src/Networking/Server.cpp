#include "Networking/Server.h"

Server::Server(const Ushort listenPort) : NetAgent(listenPort), _screen(640, 480, 640, 480) {}

void Server::Handshake(bool& connected) {
	
    // Loop until the handshake message is received
    do {
        _socket.receive_from(boost::asio::buffer(_tmpBuffer, _tmpBuffer.size()), _remoteEndpoint, 0, _errcode);
    } while(std::memcmp(_tmpBuffer.data(), HANDSHAKE_MESSAGE, HANDSHAKE_SIZE));

    // Send handshake back
    _socket.send_to(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_SIZE), _remoteEndpoint, 0, _errcode);

}

void Server::Serve() {
	
    size_t captureSize = 0;

    do {

        _screen.CaptureScreen();

        captureSize = _screen.WholeDeal(_capture);

        
    } while (Send(_capture, captureSize));
}

bool Server::Send(ByteArray bytes, size_t len) {

    // Convert the message into a list of packets
    PacketList packets = ConvertToPackets(bytes, len);

    // Loop through all the packets and send them
    for (size_t packetNo = 0; packetNo < packets.size(); packetNo++) {
        Packet& packet = packets[packetNo];

        // Send packet and then wait for acknowledgment
        _socket.send_to(boost::asio::buffer(packet.RawData(), MAX_PACKET_SIZE), _remoteEndpoint, 0, _errcode);
        _socket.receive(boost::asio::buffer(_tmpBuffer, _tmpBuffer.size()), 0, _errcode);

        if (IsDisconnectMsg()) {
            return false;
        }
        
    }
    return true;
}

void Server::AsyncSend(ByteArray const bytes, const size_t len)
{
}

void Server::AsyncReceive()
{
}

Server::~Server() {
    delete _capture;
}
