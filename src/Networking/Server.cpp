#include "Networking/Server.h"

Server::Server(const Ushort listenPort) : _screen() {

    for (int i = 0; i < Configs::VIDEO_THREADS; ++i) {
        ConnectionPtr pConnection = make_unique<Connection>(listenPort + i, true);
        connections.emplace_back(move(pConnection));
    }

    dataCon = make_unique<Connection>(listenPort + Configs::VIDEO_THREADS, true);

}

void Server::StreamVideoStream(ConnectionPtr& c, MessageReader<PacketList>& reader) {

    while (c->connected) {

        // Wait for all streams to connect
        while (Connected() && !reader.Empty()) {

            PacketList list = reader.ReadMessage();
            Send(list, c);

            c->pIO_cont->run_until(steady_clock::now() + c->timeout);
            c->pIO_cont->restart();
        }
    }
}

void Server::Receive(ConnectionPtr& pConnection) {
    const SocketPtr& pSocket = pConnection->pSocket;
    PacketBuffer& buffer = pConnection->buffer;

    pSocket->async_receive(boost::asio::buffer(buffer),
        [this, &pConnection, &buffer](const error_code& ec, const size_t size)
        {
            if (pConnection->connected && ec.value() == 0 && size > 0) {

                const Packet& packet(buffer);
                if (PacketHeader::GetType(packet.Header()) == PacketType::Mouse) {
                    //const PacketPayload& p = packet.Payload();
                    //int x = DecodeAsByte(&p.data()[0]);
                    //int y = DecodeAsByte(&p.data()[4]);
                    //SetCursorPos(x, y);
                }
                if (IsDisconnectMsg(buffer)) { Disconnect(); }
                else { Receive(pConnection); }
            }
            else {
                Disconnect();
            }
        });
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

bool Server::Listen (ConnectionPtr& pConnection) {

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

    if (streams.size() != Configs::VIDEO_THREADS) {
        
        writers.push_back(new MessageWriter<PacketList>);
        streams.push_back(new VideoStream(*writers[writers.size() - 1]));

        VideoStream* stream = streams[streams.size() - 1];
        stream->thr = thread(&Server::StreamVideoStream, this, ref(pConnection), ref(stream->reader));
    }

    return pConnection->connected;
}

bool Server::Serve() {
 
    PacketList packets = ConvertToPackets(_screen.CaptureScreen(), PacketType::Image);

    const int PACKETS_PER_THREAD = packets.size() / Configs::VIDEO_THREADS;

    // Write current screen data to each video stream
    for (int threadIndex = 0; threadIndex < Configs::VIDEO_THREADS; ++threadIndex) {
        ConnectionPtr& pConnection = connections[threadIndex];
        
        const auto& begin = packets.begin() + (threadIndex * PACKETS_PER_THREAD);
        const auto& end   = begin + PACKETS_PER_THREAD;

        writers[threadIndex]->WriteMessage(PacketList(begin, end));
    }

    return Connected();
}

Server::~Server() {
    Disconnect();
    for (auto& s : streams) { s->thr.join(); delete s; }
    for (auto& w : writers) { delete w; }
}
