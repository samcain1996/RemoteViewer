#include "Networking/Client.h"


Client::Client(const string& hostname) {
    _hostname = hostname;
    
    for (int i = 0; i < Configs::VIDEO_THREADS; ++i) {
        ConnectionPtr pConnection = make_unique<Connection>(Connection::CLIENT_BASE_PORT + i);
        connections.emplace_back(move(pConnection));
    }
}

bool Client::Connect(const Ushort remotePort, ConnectionPtr& pConnection) {
    pConnection->remotePort = remotePort;

    tcp::endpoint endpoint(address::from_string(_hostname), pConnection->remotePort);
    pConnection->pSocket->connect(endpoint, pConnection->errorcode);

    if (pConnection->errorcode.value() != 0) { return false; }

    Handshake(pConnection);

    pConnection->pIO_cont->run_until(steady_clock::now() + pConnection->timeout);
    pConnection->pIO_cont->restart();

    return true;

}

bool Client::Connect(Ushort port) {
    using namespace std::placeholders;

    vector<future<bool>> results;
    auto tryConnect = bind((bool(Client::*)(const Ushort, ConnectionPtr&))(&Client::Connect), this, _1, _2);
    transform(connections.begin(), connections.end(), back_inserter(results), [&port, &tryConnect](ConnectionPtr& pCon) {
        
        return async(tryConnect, port++, ref(pCon));
    });

    bool connected = all_of(results.begin(), results.end(), [](auto& res) { return res.get(); });

    if (connected) {
        transform(connections.begin(), connections.end(), back_inserter(threads), [this](ConnectionPtr& pCon) {
            return thread([this, &pCon]() { Receive(pCon); pCon->pIO_cont->run(); });
        });
    }

    return connected;

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

    if (packet == nullptr || size > MAX_PACKET_SIZE) { return; }

    // Full Packet
    if (packet->Header().Size() - size == 0) {
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

Client::~Client() {

    for (thread& thread : threads) { thread.join(); }
    threads.clear();

}

