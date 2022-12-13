#include "Networking/Server.h"

Server::Server(const Ushort listenPort, const std::chrono::seconds timeout) :
    NetAgent(timeout), _localport(listenPort), _screen(), _acceptor(_io_context, tcp::endpoint(tcp::v4(), listenPort))
{}

void Server::Handshake() {

    _socket.async_send(boost::asio::buffer(HANDSHAKE_MESSAGE),
        [this](const boost::system::error_code& ec, std::size_t bytesTransferred) {

            if (!ec) {
                _socket.receive(boost::asio::buffer(_tmpBuffer, HANDSHAKE_MESSAGE.size()), 0, _errcode);

                _connected = std::memcmp(_tmpBuffer.data(), OTHER_HANDSHAKE.data(), OTHER_HANDSHAKE.size()) == 0 ||
                    std::memcmp(_tmpBuffer.data(), WIN_HANDSHAKE.data(), WIN_HANDSHAKE.size()) == 0;
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

 
    PacketList packets = ConvertToPackets(_screen.CaptureScreen(), PacketTypes::Image);
    NewSend(packets);

    _io_context.run();
    _io_context.restart();

    return _connected;
}

void Server::Receive() {}

void Server::NewSend(PacketList& packets) {

    if (packets.size() <= 0) { return; }
    Packet p = packets.front();
    packets.pop();
    _socket.async_send(boost::asio::buffer(p.RawData(), p.RawData().size()),
        [this, &packets](std::error_code error, size_t bytes_transferred) {
        if (error) {
            std::cerr << "async_write: " << error.message() << std::endl;
            Disconnect();
        }

        else {

            NewSend(packets);
        }

    });

}

void Server::NewSend2(MyByte* data, size_t size) {

    if (size <= 0) { return; }

    size_t transmit_size = std::min(size, (size_t)MAX_PACKET_SIZE);

    _socket.async_write_some(boost::asio::buffer(data, transmit_size),
        [this, data = data + transmit_size, remaining = size - transmit_size, transmit_size, size]
    (std::error_code error, size_t bytes_transferred) {
            if (error) {
                std::cerr << "async_write: " << error.message() << std::endl;
                Disconnect();
            }

            else if (remaining > 0) {

                NewSend2(data, remaining);
            }

        });
}


void Server::Send(const PacketBuffer& data) {

    //PixelData image = _screen.CaptureScreen();
    //Byte* data = image.data();
    //size_t dataSize = image.size();
    //size_t transmitSize = std::min((size_t)MAX_PACKET_SIZE, dataSize);
    //boost::asio::async_write(_socket, boost::asio::buffer(data, transmitSize),
    //    [data = data + transmitSize, dataSize = dataSize - transmitSize]
    //(std::error_code error, size_t /*bytes_transferred*/) {
    //        if (error) {
    //            std::cerr << "async_write: " << error.message() << std::endl;
    //        }
    //    });
}

Server::~Server() {}
