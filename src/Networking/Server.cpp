#include "Networking/Server.h"

Server::Server(const Ushort listenPort, const std::chrono::seconds timeout) : 
    NetAgent(listenPort), _screen(), _timeout(timeout) {}

void Server::Handshake() {
   
    _socket.send_to(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_SIZE), _remoteEndpoint, 0, _errcode);

}

bool Server::Listen() {

    _socket.async_receive_from(boost::asio::buffer(_tmpBuffer, HANDSHAKE_SIZE), _remoteEndpoint,
        [&](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (ec.value() == 0 && bytes_transferred > 0) {
                _connected = std::memcmp(_tmpBuffer.data(), HANDSHAKE_MESSAGE, HANDSHAKE_SIZE) == 0;

                if (_connected) {

                    Handshake();
                }
            }
        });

    _io_context.run_one_until(std::chrono::steady_clock::now() + _timeout);

    return _connected;

}

bool Server::Serve() {

    _screen.CaptureScreen();
	
    return Send(_screen.WholeDeal());
}

bool Server::Send(const ByteVec& data) {

    // Convert the message into a list of packets
    PacketList packets = ConvertToPackets(data);

    // Loop through all the packets and send them
    for (size_t packetNo = 0; packetNo < packets.size(); packetNo++) {
        Packet& packet = packets[packetNo];

        // Send packet and then wait for acknowledgment
        _socket.send_to(boost::asio::buffer(packet.RawData(), MAX_PACKET_SIZE), _remoteEndpoint, 0, _errcode);
        _socket.receive(boost::asio::buffer(_tmpBuffer, _tmpBuffer.size()), 0, _errcode);

        if (IsDisconnectMsg()|| !_connected) {
            return false;
        }
        
    }
    return true;
}

Server::~Server() {}
