#include "Networking/Client.h"

Client::Client(const std::string& hostname, const std::chrono::seconds& timeout) : NetAgent(timeout) {
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

const bool Client::Connect(const Ushort port) {

    try {
        _socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(_hostname), port));
    }
    catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
	
    Handshake();

    _io_context.run_until(steady_clock::now() + _timeout);
    _io_context.restart();

    return true;

}

void Client::Handshake()
{
	
	_socket.async_receive(boost::asio::buffer(_tmpBuffer, HANDSHAKE_MESSAGE.size()), 
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred)
	{
        if (ec.value() == 0 && bytes_transferred > 0) {

            _socket.write_some(boost::asio::buffer(HANDSHAKE_MESSAGE, HANDSHAKE_MESSAGE.size()), _errcode);
			
            _connected = std::memcmp(_tmpBuffer.data(), HANDSHAKE_MESSAGE.data(), HANDSHAKE_MESSAGE.size()) == 0;
        }
    });
	
}

void Client::Send(const PacketBuffer& data) {}

void Client::Receive() {

	std::chrono::seconds disconnect_timout = std::chrono::seconds(2);

    while (_connected) {

        _socket.async_read_some(boost::asio::buffer(_tmpBuffer, _tmpBuffer.size()),
            [this](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (ec.value() == 0 && bytes_transferred > 0) {
					
                    _socket.write_some(boost::asio::buffer(_tmpBuffer, DISCONNECT_MESSAGE.size()), _errcode);
                    if (_errcode || Packet::InvalidPacketSize(_tmpBuffer) || IsDisconnectMsg()) {
                        Disconnect();
                        return;
                    }
					  
                    ProcessPacket(Packet(_tmpBuffer));
                }
                else { Disconnect(); }

            });

        _io_context.run_until(steady_clock::now() + disconnect_timout);
        _io_context.restart();

    }

}

Client::~Client() {}

