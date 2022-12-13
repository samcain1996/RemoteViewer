
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
using HANDSHAKE_MESSAGE = std::array<MyByte, HANDSHAKE_SIZE>;

class NetAgent {

protected:
	NetAgent(const std::chrono::seconds& timeout = std::chrono::seconds(30));

	// NetAgents shouldn't be instantiated with no arguemnts,
	// nor copied/moved from another NetAgent
	NetAgent() = delete;
	NetAgent(NetAgent&&) = delete;
	NetAgent(const NetAgent&) = delete;

	NetAgent& operator=(const NetAgent&) = delete;
	NetAgent& operator=(NetAgent&&) = delete;


	constexpr const static std::array<MyByte, HANDSHAKE_SIZE> WIN_HANDSHAKE = { 'W', 'I', 'N', '!' };
	constexpr const static std::array<MyByte, HANDSHAKE_SIZE> OTHER_HANDSHAKE = { 'N', 'O', 'T', '!' };

#if defined(_WIN32)
	constexpr const static std::array<MyByte, HANDSHAKE_SIZE>& HANDSHAKE_MESSAGE = WIN_HANDSHAKE;
#else
	constexpr const static std::array<MyByte, HANDSHAKE_SIZE> & HANDSHAKE_MESSAGE = OTHER_HANDSHAKE;
#endif

	constexpr const static std::array<MyByte, HANDSHAKE_SIZE> DISCONNECT_MESSAGE = { 'B', 'Y', 'E', '!' };

	// Random number generation
	static std::random_device rd;
	static std::mt19937 randomGenerator;

	bool _connected = false;

	std::chrono::seconds _timeout;

	boost::asio::io_context _io_context;  // Used for I/O

	std::mutex _mutex;
	tcp::socket _socket;
	boost::system::error_code _errcode;

	PacketBuffer _tmpBuffer {}; // Temporary buffer for receiving/sending packets

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	virtual PacketList ConvertToPackets(const PixelData& data, const PacketType& packetType = PacketType::Invalid);
	virtual void Handshake() = 0;
	const bool IsDisconnectMsg() const;

	virtual void Receive() = 0;
	virtual void Send(PacketList&) = 0;
	// virtual void ProcessPacket(const Packet& packet) = 0;

public:

	void Disconnect();

	const bool Connected() const;

	virtual ~NetAgent();
};