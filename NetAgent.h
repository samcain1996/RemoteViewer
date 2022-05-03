#pragma once
#include <boost/asio.hpp>
#include "Packet.h"

#define HANDSHAKE_SIZE 4

using boost::asio::ip::udp;

// Base class to Server and Client
class NetAgent {
protected:
	NetAgent(const unsigned short port) : m_port(port), m_localEndpoint(udp::v4(), port),
		m_socket(io_context, m_localEndpoint) {}

	boost::asio::io_context io_context;

	udp::endpoint m_localEndpoint, m_remoteEndpoint;

	unsigned short m_port;

	std::mutex m_mutex;

	udp::socket m_socket;
	boost::system::error_code m_errcode;

	const Byte HANDSHAKE_MESSAGE[HANDSHAKE_SIZE] = { 'H', 'I', ':', ')' };

	NetAgent(const NetAgent&&) = delete;
};