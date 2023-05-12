#include "Networking/Server.h"
#include <future>

Server::Server(const Ushort listenPort) : _screen() {

    for (int i = 0; i < VIDEO_THREADS; ++i) {
        ConnectionPtr pConnection = make_unique<Connection>(listenPort + i, true);
        connections.emplace_back(move(pConnection));
    }

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

    SocketPtr& pSocket           = pConnection->pSocket;
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

    if (pConnection->connected) { _packetGroups.push_back(PacketList()); }

}

bool Server::Serve() {
 
    PacketList packets = ConvertToPackets(_screen.CaptureScreen(), PacketType::Image);
    vector<std::future<bool>> stillConnected;
    const int PACKETS_PER_THREAD = packets.size() / VIDEO_THREADS;

    for (int threadIndex = 0; threadIndex < VIDEO_THREADS; ++threadIndex) {

        ConnectionPtr& pConnection = connections[threadIndex];

        const auto& begin = packets.begin() + (threadIndex * PACKETS_PER_THREAD);
        const auto& end   = begin + PACKETS_PER_THREAD;

        _packetGroups[threadIndex] = move(PacketList(begin, end));
        stillConnected.push_back(async(std::launch::async, &Server::ThreadFunction, 
            this, ref(_packetGroups[threadIndex]), ref(pConnection)));
    }
    
    return std::all_of(stillConnected.begin(), stillConnected.end(), [](auto& x) { return x.get(); });
}

bool Server::ThreadFunction(PacketList& packets, ConnectionPtr& pConn) {    
    Send(packets, pConn);
    pConn->pIO_cont->run_until(steady_clock::now() + pConn->timeout);
    pConn->pIO_cont->restart();
    return pConn->connected;
}

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
