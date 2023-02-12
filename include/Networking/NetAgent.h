
#pragma once

#include "Connection.h"

using HANDSHAKE_MESSAGE_T = std::array<MyByte, HANDSHAKE_SIZE>;

class NetAgent {

protected:

	NetAgent() {};

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

public:

	static bool port_in_use(unsigned short port);

protected:

	std::mutex _mutex;
	static std::random_device rd;
	static std::mt19937 randomGenerator;

	OPERATING_SYSTEM _connectedOS = OPERATING_SYSTEM::NA;

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	virtual PacketList ConvertToPackets(const PixelData& data, const PacketType& packetType = PacketType::Invalid);
	virtual void Handshake(ConnectionPtr&);
	bool IsDisconnectMsg(const PacketBuffer&) const;

	virtual void Receive(ConnectionPtr&) = 0;
	virtual void Send(PacketList&, int) = 0;


	virtual ~NetAgent() = default;

public:

	void Disconnect(ConnectionPtr&);
	bool Connected(int) const;

	// Remove this maybe
	std::vector<ConnectionPtr> connections;
};