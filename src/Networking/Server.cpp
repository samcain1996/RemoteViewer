#include "Networking/Server.h"

Server::Server(const Ushort listenPort, const std::chrono::seconds timeout) :
    NetAgent(timeout), _screen() {

    ConnectionPtr pConnection = make_unique<Connection>(listenPort, true);
    connections.emplace_back(std::move(pConnection));
}

void Server::Handshake(ConnectionPtr& pConnection) {

    // Send handshake message based on operating system
    pConnection->pSocket->async_send(boost::asio::buffer(HANDSHAKE_MESSAGE),
        [this, &pConnection](const error_code& ec, std::size_t bytesTransferred) {

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

    const SocketPtr& pSocket     = pConnection->pSocket;
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

    if (!connections[0]->connected) { return false; }
 
    PacketList packets = ConvertToPackets(_screen.CaptureScreen(), PacketType::Image);

    Send(packets, 0);
    connections[0]->pIO_cont->run_until(steady_clock::now() + connections[0]->timeout);
    connections[0]->pIO_cont->restart();
    if (!connections[0]->connected) { return false; }

    return true;
}

void Server::Receive(ConnectionPtr&) {}

void Server::Send(PacketList& packets, int idx) {

    if (packets.size() <= 0) { return; }

    const Packet& packet = packets.front();
    const auto data = packet.RawData();
    const auto size = packet.Header().Size();

    connections[idx]->pSocket->async_send(boost::asio::buffer(data, size),
        [this, &packets, idx](const error_code error, size_t bytes_transferred) {
        if (error) {
            std::cerr << "async_write: " << error.message() << std::endl;
            Disconnect(connections[idx]);
        }

        else {
            packets.erase(packets.begin());
            Send(packets, idx);
        }

    });

}


Server::~Server() {}
