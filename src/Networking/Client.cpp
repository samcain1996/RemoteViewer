#include "Networking/Client.h"

Client::Client(const Ushort port, const string& hostname) : NetAgent(port) {
    _hostname = hostname;
}

void Client::ProcessPacket(const Packet& packet) {
	
    // Get packet group
    Uint32 group = packet.Header().group;

    if (packet.Header().sequence == 0) {
        _packetGroups[group] = packet.Header().size - 1;
        return;
    }

    ThreadLock lock(_mutex);  // Prevent other threads from accessing variables used in scope

    // Add packet to list of packets in its group
    PacketPriorityQueue& packetGroupBucket = _incompletePackets[group];
    packetGroupBucket.push(packet);

	// If the packet is the last in the group, process group
    if (_packetGroups[group] == packetGroupBucket.size()) {
        
        PacketPriorityQueue* completeGroup = new PacketPriorityQueue(std::move(_incompletePackets[group]));
        _incompletePackets.erase(group);

        groupWriter->WriteMessage(completeGroup);
        _packetGroups.erase(group);
    }
}

void Client::Send(ByteArray const bytes, const size_t len) {

}

void Client::AsyncSend(ByteArray const bytes, const size_t len)
{
}

void Client::AsyncReceive()
{

    PacketBuffer packetData;

    //for (;;) {

        _socket.async_receive(boost::asio::buffer(packetData, packetData.max_size()), [&](const boost::system::error_code& ec,
            std::size_t bytes_transferred) 
{
				
                // Copy buffer to dummy packet
                ProcessPacket(Packet(packetData));

                /*if (!eventReader->Empty()) {
                    SDL_Event ev = eventReader->ReadMessage();
                    if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_QUIT) {
                        packetData[0] = '0';
                        packetData[1] = '0';
                        packetData[2] = '0';
                        packetData[3] = '0';
                    }
                    _socket.send(boost::asio::buffer(packetData, 4), 0, _errcode);
                    return;
                }*/

                
            });
        _socket.send(boost::asio::buffer(packetData, 4), 0, _errcode);
   // }
}

bool Client::Connect(const string& serverPort) {

    // Find endpoint to connect to
    udp::resolver resolver(_io_context);
    _remoteEndpoint = *resolver.resolve(udp::v4(), _hostname, serverPort).begin();

    // Handshake
    Byte handshakeBuf[HANDSHAKE_SIZE];
    _socket.connect(_remoteEndpoint);

    _socket.send(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_SIZE), 0, _errcode);
    _socket.receive(boost::asio::buffer(handshakeBuf, HANDSHAKE_SIZE), 0, _errcode);

    _connected = std::memcmp(handshakeBuf, HANDSHAKE_MESSAGE, HANDSHAKE_SIZE) == 0;


    return _connected;
}

void Client::Receive() {
   
    PacketBuffer packetData;

    while (_connected) {

        
        // Receive packet
        _socket.receive(boost::asio::buffer(packetData, packetData.max_size()), 0, _errcode);

        _socket.send(boost::asio::buffer(packetData, 4), 0, _errcode);

        // Copy buffer to dummy packet
        ProcessPacket(Packet(packetData));

    }

}

Client::~Client() {
}

