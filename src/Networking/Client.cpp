#include "Networking/Client.h"

Client::Client(const std::string& hostname, const std::chrono::seconds& timeout) : NetAgent(timeout) {
    _hostname = hostname;
}

const void Client::Connect(const Ushort port, const std::function<void()>& onConnect) {

    try {
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(_hostname), port);
        _socket.connect(endpoint);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;

        _io_context.reset();
        _socket = tcp::socket(_io_context);
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

                NetAgent::Handshake();
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
                
                Process(_tmpBuffer, bytes_transferred);
                if (IsDisconnectMsg()) { Disconnect(); }
                else {
                    Receive();
                }
            }
            else {
                Disconnect();
            }
        });

}

void Client::Process(const PacketBuffer& buf, int size) {
    static std::optional<std::pair<PacketBuffer, int>> current = std::nullopt;
    
    const Packet packet(buf);
    const bool hasHeader = Packet::VerifyPacket(packet);

    if (size == MAX_PACKET_SIZE && !hasHeader) { return; }

    if (hasHeader && packet.Header().Size() - size == 0) {
        current = std::nullopt;
        msgWriter->WriteMessage(std::make_shared<Packet>(buf));
        return;
    }

    if (current.has_value()) {
        if (hasHeader) {
            current = { buf, packet.Header().Size() };
        }
        else {
            const Packet& currentPacket = current.value().first;
            const int remaining = currentPacket.Header().Size() - current.value().second - size;
            if (remaining == 0) { 
                std::copy(buf.begin(), buf.begin() + size,
                    current.value().first.begin() + current.value().second);
                msgWriter->WriteMessage(std::make_shared<Packet>(std::move(Packet(current.value().first)))); 
            }
            else if (remaining > 0) {
                std::copy(buf.begin(), buf.begin() + size,
                    current.value().first.begin() + current.value().second);
            }
        }
    }
    else {

        if (hasHeader) {
            int remaining = packet.Header().Size() - size;
            if (remaining == 0) { msgWriter->WriteMessage(std::make_shared<Packet>(std::move(Packet(buf)))); }
            else {
                current = { buf, packet.Header().Size() };
            }
        }
    }
}

Client::~Client() {}

