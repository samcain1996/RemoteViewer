
#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <random>
#include <iostream>
#include "Networking/Packet.h"
#include "Messages.h"

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;
using boost::system::error_code;
using std::chrono::steady_clock;
using std::chrono::seconds;
using std::make_unique;

constexpr const int HANDSHAKE_SIZE = 4;
constexpr const int IMAGE_THREADS = 1;

using HANDSHAKE_MESSAGE_T = std::array<MyByte, HANDSHAKE_SIZE>;

using SocketPtr = std::unique_ptr<tcp::socket>;
using IOContPtr = std::unique_ptr<io_context>;
using AccptrPtr = std::unique_ptr<tcp::acceptor>;

struct Connection {

	static constexpr inline Ushort DEFUALT_PORT	      = 20000;
	static constexpr inline Ushort SERVER_PORT_OFFSET = 1000;
	static constexpr inline Ushort CLIENT_PORT_OFFSET = 2000;

	bool connected = false;
	seconds timeout { 30 };
	PacketBuffer buffer {};
	error_code errorcode {};

	Ushort localPort;
	Ushort remotePort;

	IOContPtr pIO_cont;
	SocketPtr pSocket;
	AccptrPtr pAcceptor;

	Connection(const Ushort localPort = 0, const bool listen = false) : localPort(localPort) {

		pIO_cont = make_unique<io_context>();
		pSocket  = make_unique<tcp::socket>(*pIO_cont);

		if (listen) {
			pAcceptor = make_unique<tcp::acceptor>(*pIO_cont, tcp::endpoint(tcp::v4(), localPort));
		}
	};

	Connection(const Connection&) = delete;
	Connection(Connection&&) = delete;

	~Connection() {};
};

using ConnectionPtr = std::unique_ptr<Connection>;

class NetAgent {

protected:

	NetAgent(const seconds& timeout = seconds(30));

	// NetAgents shouldn't be instantiated with no arguemnts,
	// nor copied/moved from another NetAgent
	NetAgent(NetAgent&&) noexcept = delete;
	NetAgent(const NetAgent&) = delete;

	NetAgent& operator=(const NetAgent&) = delete;
	NetAgent& operator=(NetAgent&&) = delete;


	constexpr const static HANDSHAKE_MESSAGE_T WIN_HANDSHAKE = { 'W', 'I', 'N', '!' };
	constexpr const static HANDSHAKE_MESSAGE_T MAC_HANDSHAKE = { 'M', 'A', 'C', '!' };
	constexpr const static HANDSHAKE_MESSAGE_T LIN_HANDSHAKE = { 'L', 'N', 'X', '!' };

#if defined(_WIN32)
	constexpr const static HANDSHAKE_MESSAGE_T& HANDSHAKE_MESSAGE = WIN_HANDSHAKE;
#elif defined(__APPLE__)
	constexpr const static HANDSHAKE_MESSAGE_T& HANDSHAKE_MESSAGE = MAC_HANDSHAKE;
#else 
	constexpr const static HANDSHAKE_MESSAGE_T& HANDSHAKE_MESSAGE = LIN_HANDSHAKE;
#endif

	constexpr const static HANDSHAKE_MESSAGE_T DISCONNECT_MESSAGE = { 'B', 'Y', 'E', '!' };

	// Random number generation
	static std::random_device rd;
	static std::mt19937 randomGenerator;

	OPERATING_SYSTEM _connectedOS = OPERATING_SYSTEM::NA;
public:
	std::vector<ConnectionPtr> connections;
protected:
	std::mutex _mutex;

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	virtual PacketList ConvertToPackets(const PixelData& data, const PacketType& packetType = PacketType::Invalid);
	virtual void Handshake(ConnectionPtr&);
	bool IsDisconnectMsg(const PacketBuffer&) const;

	virtual void Receive(ConnectionPtr&) = 0;
	virtual void Send(PacketList&, int) = 0;

public:
	void Disconnect(ConnectionPtr&);

	bool Connected(int) const;

	static bool port_in_use(unsigned short port) {
		using namespace boost::asio;
		using ip::tcp;

		io_service svc;
		tcp::acceptor a(svc);

		error_code ec;
		a.open(tcp::v4(), ec) || a.bind({ tcp::v4(), port }, ec);

		return ec == error::address_in_use;
	}

	virtual ~NetAgent() = default;
};