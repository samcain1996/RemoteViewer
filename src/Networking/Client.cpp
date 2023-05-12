#include "Networking/Client.h"

Client::Client(const string& hostname) {
    _hostname = hostname;
    
    for (int i = 0; i < VIDEO_THREADS; ++i) {
        ConnectionPtr pConnection = make_unique<Connection>(Connection::CLIENT_BASE_PORT + i);
        connections.emplace_back(move(pConnection));
    }
}

const void Client::Connect(const Ushort remotePort, const Action& onConnect) {

    static int idx = 0;
    ConnectionPtr& pConnection = connections[idx++];
    pConnection->remotePort = remotePort;

    tcp::endpoint endpoint(address::from_string(_hostname), pConnection->remotePort);
    pConnection->pSocket->connect(endpoint, pConnection->errorcode);

    if (pConnection->errorcode.value() != 0) { std::terminate(); }
   
    Handshake(pConnection);

    pConnection->pIO_cont->run_until(steady_clock::now() + pConnection->timeout);
    pConnection->pIO_cont->restart();

    if (pConnection->connected) { onConnect(); }


}

void Client::Handshake(ConnectionPtr& pConnection) {

    const auto& pSocket = pConnection->pSocket;
    auto& buffer = pConnection->buffer;

    pSocket->async_receive(boost::asio::buffer(buffer, HANDSHAKE_MESSAGE.size()),
        [this, &pSocket, &pConnection](const error_code& ec, const size_t size)
        {
            if (ec.value() == 0 && size > 0) {

                pSocket->write_some(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_MESSAGE.size()), pConnection->errorcode);

                NetAgent::Handshake(pConnection);
            }
        });

}

void Client::Send(PacketList& data, ConnectionPtr& pConnection) {}

void Client::Receive(ConnectionPtr& pConnection) {

    const SocketPtr& pSocket = pConnection->pSocket;
    PacketBuffer& buffer    = pConnection->buffer;

    pSocket->async_receive(boost::asio::buffer(buffer),
        [this, &pConnection, &buffer](const error_code& ec, const size_t size)
        {
            if (pConnection->connected && ec.value() == 0 && size > 0) {
                //msgWriter->WriteMessage(make_shared<Packet>(buffer));
                AdjustForPacketLoss(buffer, size);
                if (IsDisconnectMsg(buffer)) { Disconnect(); }
                else { Receive(pConnection); }
            }
            else {
                Disconnect();
            }
        });

}


// TODO: CLEAN THIS UP
void Client::AdjustForPacketLoss(const PacketBuffer& buf, const int size) {

    static std::optional<std::pair<PacketBuffer, int>> current = std::nullopt;
    
    const PacketPtr packet = Packet::VerifyPacket(buf);

    if ( packet == nullptr || size > MAX_PACKET_SIZE ) { return; }

    // Full Packet
    if ( packet->Header().Size() - size == 0 ) {
        current = std::nullopt;
        msgWriter->WriteMessage(make_shared<Packet>(move(Packet(buf))));
        return;
    }

    // Split Packet
    if (current.has_value()) {

        const Packet& currentPacket = current.value().first;
        const int remaining = currentPacket.Header().Size() - current.value().second - size;

        if (remaining == 0) { 
            std::copy(buf.begin(), buf.begin() + size,
                current.value().first.begin() + current.value().second);
            msgWriter->WriteMessage(make_shared<Packet>(move(Packet(current.value().first)))); 
        }

        else if (remaining > 0) {
            std::copy(buf.begin(), buf.begin() + size,
                current.value().first.begin() + current.value().second);
        }

        return;
    }

    int remaining = packet->Header().Size() - size;
    if (remaining == 0) { msgWriter->WriteMessage(make_shared<Packet>(move(Packet(buf)))); }
    else {
        current = { buf, packet->Header().Size() };
    }
}

Client::~Client() {}

