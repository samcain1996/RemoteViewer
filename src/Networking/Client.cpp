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

bool Client::Send(ByteArray const bytes, const size_t len) {
    SendDisconnect();
    return true;
}

void Client::AsyncSend(ByteArray const bytes, const size_t len)
{
}

void Client::AsyncReceive() {

    /* Broken right now */
     
    
    while (_connected) {

        _io_context.restart();
        _socket.async_receive(boost::asio::buffer(_tmpBuffer, _tmpBuffer.max_size()), [&](const boost::system::error_code& ec,
            std::size_t bytes_transferred)
            {
                if (!ec && bytes_transferred > 0) {

                    if (IsDisconnectMsg() || !_connected) {
                        _connected = false;
                        _io_context.stop();
                        return;
                    }
                    // Copy buffer to dummy packet
                    ProcessPacket(Packet(_tmpBuffer));
                    _socket.send(boost::asio::buffer(_tmpBuffer, 4), 0, _errcode);
                }

            });

        _io_context.run_one();
		
    }

}

bool Client::Connect(const string& serverPort) {

    // Find endpoint to connect to
    udp::resolver resolver(_io_context);
    _remoteEndpoint = *resolver.resolve(udp::v4(), _hostname, serverPort).begin();

    _socket.connect(_remoteEndpoint);

    return true;

}

void Client::Handshake(bool& connected)
{

    _socket.send(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_SIZE), 0, _errcode);

	_socket.async_receive(boost::asio::buffer(_tmpBuffer, HANDSHAKE_SIZE), 0, [&](const boost::system::error_code& ec, std::size_t bytes_transferred)
	{
        if (ec.value() == 0 && bytes_transferred > 0) {
            connected = _connected = std::memcmp(_tmpBuffer.data(), HANDSHAKE_MESSAGE, HANDSHAKE_SIZE) == 0;
        }
	});


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

