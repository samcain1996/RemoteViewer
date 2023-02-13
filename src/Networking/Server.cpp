#include "Networking/Server.h"

Server::Server(const Ushort listenPort) : _screen() {

    ConnectionPtr pConnection = make_unique<Connection>(listenPort, true);
    connections.emplace_back(std::move(pConnection));
}

void Server::Handshake(ConnectionPtr& pConnection) {

    // Send handshake message based on operating system
    pConnection->pSocket->async_send(boost::asio::buffer(HANDSHAKE_MESSAGE),
        [this, &pConnection](const error_code& ec, const size_t bytesTransferred) {

            if (!ec) {
                
                // If successfully send, read response and perform handhsake
                pConnection->pSocket->receive(boost::asio::buffer(pConnection->buffer, HANDSHAKE_MESSAGE.size()),
                    0, pConnection->errorcode);
                
                NetAgent::Handshake(pConnection);
            }
        }
    );

}

void Server::Listen (ConnectionPtr& pConnection) {

    SocketPtr& pSocket     = pConnection->pSocket;
    const AccptrPtr& pAcceptor   = pConnection->pAcceptor;
    const IOContPtr& pIO_context = pConnection->pIO_cont;

    pAcceptor->async_accept(*pSocket, [this, &pConnection](const error_code& ec)
        {
            if (!ec) {
                Handshake(pConnection);
            }
        });

    pIO_context->run_until(steady_clock::now() + pConnection->timeout);
    pIO_context->restart();

}

bool Server::Serve() {
 
    PacketList packets = ConvertToPackets(_screen.CaptureScreen(), PacketType::Image);
    const int PACKETS_PER_THREAD = packets.size() / SEND_THREADS;

    for (int threadIndex = 0; threadIndex < SEND_THREADS; ++threadIndex) {

        ConnectionPtr& pConnection = connections[threadIndex];

        const auto& begin = packets.begin() + (threadIndex * PACKETS_PER_THREAD);
        const auto& end = (threadIndex == SEND_THREADS - 1) ? packets.end() : packets.begin() + (threadIndex * PACKETS_PER_THREAD);

        PacketList list(begin, end);

        Send(list, pConnection);

        pConnection->pIO_cont->run_until(steady_clock::now() + pConnection->timeout);
        pConnection->pIO_cont->restart();

        if (!pConnection->connected) { return false; }
    }

    return true;
}

void Server::Receive(ConnectionPtr& pConnection) {}

void Server::Send(PacketList& packets, ConnectionPtr& pConnection) {

    if (packets.size() <= 0) { return; }

    const Packet& packet = packets.front();

    pConnection->pSocket->async_send(boost::asio::buffer(packet.RawData(), packet.Header().Size()),
        [this, &packets, &pConnection](const error_code& error, const size_t size) {
        if (error) {
            log.WriteLine("async_write: " + error.message());
            Disconnect();
        }

        else {
            packets.erase(packets.begin());
            Send(packets, pConnection);
        }

    });

}


Server::~Server() {}
