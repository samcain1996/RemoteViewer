#include "Networking/Client.h"

Client::Client(const std::string& hostname) {
    _hostname = hostname;
    
    ConnectionPtr pConnection = make_unique<Connection>(Connection::BASE_PORT + Connection::CLIENT_PORT_OFFSET);
    connections.emplace_back(std::move(pConnection));
}

const void Client::Connect(const Ushort remotePort, const Action& onConnect) {

    const int idx = connections.size() - 1;
    ConnectionPtr& pConnection = connections[idx];
    pConnection->remotePort = remotePort;

    try {
        tcp::endpoint endpoint(address::from_string(_hostname), pConnection->remotePort);
        pConnection->pSocket->connect(endpoint);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;

        pConnection->pIO_cont->reset();
        pConnection->pSocket.reset(new tcp::socket(*(pConnection->pIO_cont)));
        return;
    }
   
    Handshake(pConnection);

    pConnection->pIO_cont->run_until(steady_clock::now() + pConnection->timeout);
    pConnection->pIO_cont->restart();

    if (pConnection->connected) { onConnect(); }

}

void Client::Handshake(ConnectionPtr& pConnection) {

    const auto& pSocket = pConnection->pSocket;
    auto& buffer = pConnection->buffer;

    pSocket->async_receive(boost::asio::buffer(buffer, HANDSHAKE_MESSAGE.size()),
        [this, &pSocket, &pConnection](const error_code& ec, std::size_t bytes_transferred)
        {
            if (ec.value() == 0 && bytes_transferred > 0) {

                pSocket->write_some(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_MESSAGE.size()), pConnection->errorcode);

                NetAgent::Handshake(pConnection);
            }
        });

}

void Client::Send(PacketList& data, int idx) {}

void Client::Start(ConnectionPtr& pConnection) {

    Receive(pConnection);
    pConnection->pIO_cont->run();
}

void Client::Receive(ConnectionPtr& pConnection) {

    const SocketPtr& pSocket = pConnection->pSocket;
    PacketBuffer& buffer    = pConnection->buffer;

    pSocket->async_receive(boost::asio::buffer(buffer),
        [this, &pConnection, &buffer](const error_code& ec, std::size_t bytes_transferred)
        {
            if (ec.value() == 0 && bytes_transferred > 0)  {

                Process(buffer, bytes_transferred);
                if (IsDisconnectMsg(buffer)) { Disconnect(pConnection); }
                else { Receive(pConnection); }
            }
            else {
                Disconnect(pConnection);
            }
        });

}

// TODO: CLEAN THIS UP
void Client::Process(const PacketBuffer& buf, int size) {
    static std::optional<std::pair<PacketBuffer, int>> current = std::nullopt;
    
    const PacketPtr packet = Packet::VerifyPacket(buf);

    if (size == MAX_PACKET_SIZE && packet == nullptr) { return; }

    if (packet != nullptr && packet->Header().Size() - size == 0) {
        current = std::nullopt;
        msgWriter->WriteMessage(std::make_shared<Packet>(std::move(Packet(buf))));
        return;
    }

    if (current.has_value()) {
        if (packet != nullptr) {
            current = { buf, packet->Header().Size() };
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
        return;
    }


        if (packet != nullptr) {
            int remaining = packet->Header().Size() - size
;
            if (remaining == 0) { msgWriter->WriteMessage(std::make_shared<Packet>(std::move(Packet(buf)))); }
            else {
                current = { buf, packet->Header().Size() };
            }
        }
}

Client::~Client() {}

