#include "Networking/Server.h"

Server::Server(const Ushort listenPort, const std::chrono::seconds timeout) :
    NetAgent(timeout), _localport(listenPort), _screen(), _acceptor(_io_context, tcp::endpoint(tcp::v4(), listenPort))
{}

void Server::Handshake() {

    _socket.async_send(boost::asio::buffer(HANDSHAKE_MESSAGE),
        [this](const boost::system::error_code& ec, std::size_t bytesTransferred) {

            if (!ec) {

                _socket.receive(boost::asio::buffer(_tmpBuffer, HANDSHAKE_MESSAGE.size()), 0, _errcode);

                NetAgent::Handshake();
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

    if (!_connected) { return false; }
 
    PacketList packets = ConvertToPackets(_screen.CaptureScreen(), PacketType::Image);
    Send(packets);

    _io_context.run();
    _io_context.restart();

    return _connected;
}

void Server::Receive() {}

void Server::Send(PacketList& packets) {

    if (packets.size() <= 0) { return; }

    const Packet& packet = packets.front();
    const auto data = packet.RawData();
    const auto size = packet.Header().Size();

    _socket.async_send(boost::asio::buffer(data, size),
        [this, &packets](std::error_code error, size_t bytes_transferred) {
        if (error) {
            std::cerr << "async_write: " << error.message() << std::endl;
            Disconnect();
        }

        else {
            packets.erase(packets.begin());
            Send(packets);
        }

    });

}


Server::~Server() {}
