#include "Networking/Server.h"

Server::Server(const Ushort listenPort, const std::chrono::seconds timeout) : 
    NetAgent(timeout), _localport(listenPort), _screen(), _acceptor(_io_context, tcp::endpoint(tcp::v4(), listenPort))
{}

void Server::Handshake() {
   
    _socket.async_send(boost::asio::buffer(HANDSHAKE_MESSAGE),
        [&](const boost::system::error_code& ec, std::size_t bytesTransferred) {

            if (!ec) {
                _socket.receive(boost::asio::buffer(_tmpBuffer, HANDSHAKE_MESSAGE.size()), 0, _errcode);

				_connected = std::memcmp(_tmpBuffer.data(), HANDSHAKE_MESSAGE.data(), HANDSHAKE_MESSAGE.size()) == 0;
            }
        }
    );

}

void Server::Listen() {

    _acceptor.async_accept(_socket, [this](boost::system::error_code ec) 
        {
            if (!ec) {
                Handshake();
            }
        });

    _io_context.run_until(std::chrono::steady_clock::now() + _timeout);
    _io_context.restart();

}

bool Server::Serve() {
    // Convert the message into a list of packets
    PacketList packets = ConvertToPackets(_screen.CaptureScreen());

    // Loop through all the packets and send them
    for (size_t packetNo = 0; packetNo < packets.size(); packetNo++) {
        Packet& packet = packets[packetNo];

        Send(ByteVec(packet.RawData().data(), packet.RawData().data() + packet.Header().size));
    }
    return true;
}

void Server::Receive() {
	_socket.async_read_some(boost::asio::buffer(_tmpBuffer, DISCONNECT_MESSAGE.size()), 
        [&](const boost::system::error_code& ec, std::size_t bytesTransferred) {
		if (!ec) {
            if (IsDisconnectMsg()) {
                _connected = false;
            }
		}
	});
}

bool Server::Send(const ByteVec& data) {

    // Send packet and then wait for acknowledgment
    _socket.async_write_some(boost::asio::buffer(data, _tmpBuffer.size()),
        [&](const boost::system::error_code& ec, std::size_t bytesTransferred) {
    
            if (!ec) {
                Receive();
    
            }
        }
    );
    
    _io_context.run();
    _io_context.restart();

    return true;
}

Server::~Server() {
}
