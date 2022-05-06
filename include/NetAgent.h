#pragma once
#include <boost/asio.hpp>
#include <fstream>
#include "Packet.h"

using boost::asio::ip::udp;

// Base class to Server and Client
class NetAgent {
protected:
	NetAgent(const unsigned short port) : _port(port), _localEndpoint(udp::v4(), port),
		_socket(_io_context, _localEndpoint) {};
	
	// NetAgents shouldn't be instantiated with no arguemnts,
	// nor copied/moved from another NetAgent
	NetAgent() 				  = delete;
	NetAgent(NetAgent&&) 	  = delete;
	NetAgent(const NetAgent&) = delete;

	boost::asio::io_context _io_context;  // Used for I/O
	unsigned short _port;				  // Port to reside on
	udp::endpoint _localEndpoint, _remoteEndpoint;

	std::mutex _mutex;
	udp::socket _socket;
	boost::system::error_code _errcode;

	const static ushort HANDSHAKE_SIZE = 4;
	const static Byte HANDSHAKE_MESSAGE[HANDSHAKE_SIZE];
};

inline const Byte NetAgent::HANDSHAKE_MESSAGE[HANDSHAKE_SIZE] = {'H', 'I', ':', ')'};