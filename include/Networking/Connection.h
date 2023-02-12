#pragma once

#include <boost/asio.hpp>
#include <thread>
#include <random>
#include <iostream>
#include "Networking/Packet.h"
#include "Messages.h"

using std::chrono::seconds;
using std::chrono::steady_clock;
using boost::system::error_code;
using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

using std::make_unique;

constexpr const int IMAGE_THREADS = 1;
constexpr const int SEND_THREADS = 1;  // KEEP AT 1, WILL BREAK AT ANYTHING ELSE

using SocketPtr = std::unique_ptr<tcp::socket>;
using IOContPtr = std::unique_ptr<io_context>;
using AccptrPtr = std::unique_ptr<tcp::acceptor>;

struct Connection {

	static constexpr inline Ushort BASE_PORT = 20000;
	static constexpr inline Ushort SERVER_BASE_PORT = BASE_PORT + 1000;
	static constexpr inline Ushort CLIENT_BASE_PORT = BASE_PORT + 2000;

	seconds timeout { 30 };
	bool connected = false;

	Ushort remotePort = 0;
	Ushort localPort  = 0;

	PacketBuffer buffer  {};
	error_code errorcode {};

	IOContPtr pIO_cont;
	SocketPtr pSocket;
	AccptrPtr pAcceptor;

	Connection(const Ushort localPort = 0, const bool listen = false) {

		pIO_cont = make_unique<io_context>();
		pSocket = make_unique<tcp::socket>(*pIO_cont);

		if (listen) {
			pAcceptor = make_unique<tcp::acceptor>(*pIO_cont, tcp::endpoint(tcp::v4(), localPort));
		}
	};

	Connection(const Connection&) = delete;
	Connection(Connection&&) = delete;

	~Connection() {};
};

using ConnectionPtr = std::unique_ptr<Connection>;
using ConnectionList = std::vector<ConnectionPtr>;