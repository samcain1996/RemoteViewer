
#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <random>
#include <iostream>
#include "Networking/Packet.h"
#include "Messages.h"

using boost::asio::ip::tcp;
using std::chrono::steady_clock;

constexpr const int HANDSHAKE_SIZE = 4;
using HANDSHAKE_MESSAGE_T = std::array<MyByte, HANDSHAKE_SIZE>;


class NetAgent {

protected:
	NetAgent(const std::chrono::seconds& timeout = std::chrono::seconds(30));

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
#else
	constexpr const static HANDSHAKE_MESSAGE_T& HANDSHAKE_MESSAGE = OTHER_HANDSHAKE;
#endif

	constexpr const static HANDSHAKE_MESSAGE_T DISCONNECT_MESSAGE = { 'B', 'Y', 'E', '!' };

	// Random number generation
	static std::random_device rd;
	static std::mt19937 randomGenerator;

	bool _connected = false;
	OPERATING_SYSTEM _connectedOS = OPERATING_SYSTEM::NA;
	std::chrono::seconds _timeout;

	boost::asio::io_context _io_context;  // Used for I/O

	std::mutex _mutex;
	tcp::socket _socket;
	boost::system::error_code _errcode;

	PacketBuffer _tmpBuffer {}; // Temporary buffer for receiving/sending packets

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	virtual PacketList ConvertToPackets(const PixelData& data, const PacketType& packetType = PacketType::Invalid);
	virtual void Handshake();
	bool IsDisconnectMsg() const;

	virtual void Receive() = 0;
	virtual void Send(PacketList&) = 0;
	// virtual void ProcessPacket(const Packet& packet) = 0;

public:

	void Disconnect();

	bool Connected() const;
	OPERATING_SYSTEM ConnectedOS() const;

	static bool port_in_use(unsigned short port) {
		using namespace boost::asio;
		using ip::tcp;

		io_service svc;
		tcp::acceptor a(svc);

		boost::system::error_code ec;
		a.open(tcp::v4(), ec) || a.bind({ tcp::v4(), port }, ec);

		return ec == error::address_in_use;
	}

	virtual ~NetAgent() = default;
};