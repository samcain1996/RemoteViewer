#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <random>
#include "Networking/Packet.h"
#include "Messages.h"

using boost::asio::ip::udp;

class NetAgent {

protected:
	NetAgent(const Ushort localPort);
	
	// NetAgents shouldn't be instantiated with no arguemnts,
	// nor copied/moved from another NetAgent
	NetAgent() 				  = delete;
	NetAgent(NetAgent&&) 	  = delete;
	NetAgent(const NetAgent&) = delete;

	NetAgent& operator=(const NetAgent&) = delete;
	NetAgent& operator=(NetAgent&&) = delete;

	constexpr const static Ushort HANDSHAKE_SIZE = 4;
	constexpr const static Byte HANDSHAKE_MESSAGE[HANDSHAKE_SIZE] = { 'H', 'I', ':', ')' };

	constexpr const static Ushort DISCONNECT_SIZE = 4;
	constexpr const static Byte DISCONNECT_MESSAGE[DISCONNECT_SIZE] = { 'B', 'Y', 'E', '!' };
	
	static std::random_device rd;  // Used to seed random number generator

	// Random number generator, C-style rand does not have enough precision
	static std::mt19937 randomGenerator;

	boost::asio::io_context _io_context;  // Used for I/O
	Ushort _localPort;				  // Port to reside on
	udp::endpoint _localEndpoint, _remoteEndpoint;

	std::mutex _mutex;
	udp::socket _socket;
	boost::system::error_code _errcode;

	PacketBuffer _tmpBuffer; // Temporary buffer for receiving/sending packets

	// A map that maps packet groups to a priority queue
	PacketGroupPriorityQueueMap _incompletePackets;

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	virtual PacketList ConvertToPackets(ByteArray& bytes, size_t len);
	virtual void Handshake(bool& connected) = 0;
	virtual const bool IsDisconnectMsg() const;
	
	virtual void Receive() = 0;
	virtual bool Send(ByteArray const bytes, const size_t len) = 0;
	virtual void ProcessPacket(const Packet&) = 0;

	virtual void AsyncSend(ByteArray const bytes, const size_t len) = 0;
	virtual void AsyncReceive() = 0;

	virtual void SendDisconnect() { _socket.send(boost::asio::buffer(DISCONNECT_MESSAGE, DISCONNECT_SIZE), 0, _errcode); }

public:
	
	virtual ~NetAgent();
};