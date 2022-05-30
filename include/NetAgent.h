#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <random>
#include "Packet.h"

using boost::asio::ip::udp;

// Base class to Server and Client
class NetAgent {
protected:
	NetAgent(const unsigned short port) : _port(port), _localEndpoint(udp::v4(), port),
		_socket(_io_context, _localEndpoint), _keepAlive(true) {};
	
	// NetAgents shouldn't be instantiated with no arguemnts,
	// nor copied/moved from another NetAgent
	NetAgent() 				  = delete;
	NetAgent(NetAgent&&) 	  = delete;
	NetAgent(const NetAgent&) = delete;

	virtual ~NetAgent() {};

	NetAgent& operator=(const NetAgent&) = delete;
	NetAgent& operator=(NetAgent&&) = delete;
	
	static 	std::random_device rd;  // Used to seed random number generator

	// Random number generator, C-style rand does not have enough precision
	static std::mt19937 randomGenerator;

	boost::asio::io_context _io_context;  // Used for I/O
	unsigned short _port;				  // Port to reside on
	udp::endpoint _localEndpoint, _remoteEndpoint;

	std::mutex _mutex;
	udp::socket _socket;
	boost::system::error_code _errcode;

	// A map that maps packet groups to a priority queue
	PacketGroupPriorityQueueMap _incompletePackets;
	PacketGroupMap _packetGroups;

	std::atomic<bool> _keepAlive;

	virtual void Receive() = 0;
	virtual void Send(ByteArray bytes, size_t len) = 0;
	virtual void ProcessPacket(const Packet&) = 0;

	constexpr const static Ushort HANDSHAKE_SIZE = 4;
	constexpr const static Byte HANDSHAKE_MESSAGE[HANDSHAKE_SIZE] = { 'H', 'I', ':', ')' };

	constexpr const static Ushort DISCONNECT_SIZE = 4;
	constexpr const static Byte DISCONNECT_MESSAGE[DISCONNECT_SIZE] = {'B', 'Y', 'E', '!' };
};