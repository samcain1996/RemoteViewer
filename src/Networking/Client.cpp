#include "Networking/Client.h"

Client::Client(const std::string& hostname, const std::chrono::seconds& timeout) : NetAgent(timeout) {
    _hostname = hostname;

}

const void Client::Connect(const Ushort port, const std::function<void()>& onConnect) {

    try {
        _socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(_hostname), port));
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    Handshake();

    _io_context.run_until(steady_clock::now() + _timeout);
    _io_context.restart();

    if (_connected) { onConnect(); }

}

void Client::Handshake() {

    _socket.async_receive(boost::asio::buffer(_tmpBuffer, HANDSHAKE_MESSAGE.size()),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            if (ec.value() == 0 && bytes_transferred > 0) {

                _socket.write_some(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_MESSAGE.size()), _errcode);

                _connected = std::memcmp(_tmpBuffer.data(), OTHER_HANDSHAKE.data(), OTHER_HANDSHAKE.size()) == 0 ||
                    std::memcmp(_tmpBuffer.data(), WIN_HANDSHAKE.data(), WIN_HANDSHAKE.size()) == 0;
            }
        });

}

void Client::Send(PacketList& data) {}

void Client::Start() {

    Receive();
    _io_context.run();
}

void Client::Receive() {
    _socket.async_receive(boost::asio::buffer(_tmpBuffer),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            if (ec.value() == 0 && bytes_transferred > 0 && _connected) {
                groupWriter->WriteMessage(new Packet(_tmpBuffer));
                Receive();
            }
            else {
                Disconnect();
            }
        });

}

Client::~Client() {}

