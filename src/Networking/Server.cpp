#include "Networking/Server.h"

Server::Server(const Ushort listenPort, const std::chrono::seconds timeout) : 
    NetAgent(timeout), _localport(listenPort), _screen(), _acceptor(_io_context, tcp::endpoint(tcp::v4(), listenPort))
{}

void Server::Handshake(bool& isWindows) {
   
    _socket.async_send(boost::asio::buffer(HANDSHAKE_MESSAGE),
        [this, &isWindows](const boost::system::error_code& ec, std::size_t bytesTransferred) {

            if (!ec) {
                _socket.receive(boost::asio::buffer(_tmpBuffer, HANDSHAKE_MESSAGE.size()), 0, _errcode);

                _connected = std::memcmp(_tmpBuffer.data(), OTHER_HANDSHAKE.data(), OTHER_HANDSHAKE.size()) == 0 ||
                    std::memcmp(_tmpBuffer.data(), WIN_HANDSHAKE.data(), WIN_HANDSHAKE.size()) == 0;

                isWindows = std::memcmp(_tmpBuffer.data(), WIN_HANDSHAKE.data(), WIN_HANDSHAKE.size()) == 0;
            }
        }
    );

}

void Server::Listen() {

    _acceptor.async_accept(_socket, [this](boost::system::error_code ec) 
        {
            bool dummy = false;
            if (!ec) {
                Handshake(dummy);
            }
        });

    _io_context.run_until(std::chrono::steady_clock::now() + _timeout);
    _io_context.restart();

}

bool Server::Serve() {

    // Convert the message into a list of packets
    // PacketList packets = ConvertToPackets(_screen.CaptureScreen(), PacketTypes::Image);

    // Loop through all the packets and send them
    //for (size_t packetNo = 0; packetNo < packets.size() && _connected; packetNo++) {
    //    const Packet& packet = packets[packetNo];

    //    Send(packet.RawData());
    //}

    ImageData image = _screen.CaptureScreen();
    NewSend(image.data(), image.size());

    _io_context.run();
    _io_context.restart();

    return _connected;
}

void Server::Receive() {}

void Server::NewSend(Byte* data, size_t size) {

    if (size == 0 || size > ScreenCapture::CalculateBMPFileSize()) {
        auto tmp = _screen.CaptureScreen();
        data = tmp.data();
        size = tmp.size();
    }

    size_t transmitSize = std::min((size_t)MAX_PACKET_SIZE, size);

    boost::asio::async_write(_socket, boost::asio::buffer(data, transmitSize),
        [this, data = data + transmitSize, size = size - transmitSize]
    (std::error_code error, size_t /*bytes_transferred*/) {
            if (error) {
                std::cerr << "async_write: " << error.message() << std::endl;
            }
            else { NewSend(data, size); }
        });
}

void Server::Send(const PacketBuffer& data) {

    //ImageData image = _screen.CaptureScreen();
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
