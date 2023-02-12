
#pragma once

#include "Networking/Connection.h"

constexpr const int HANDSHAKE_SIZE = 4;
using HANDSHAKE_MESSAGE_T = std::array<MyByte, HANDSHAKE_SIZE>;

class NetAgent {

public:
	// Remove this maybe
	ConnectionList connections;

protected:

	static inline std::mt19937 randomGenerator{ std::random_device()() };
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

	static bool portAvailable(unsigned short port);

protected:

	NetAgent() {};

	// NetAgents shouldn't be instantiated with no arguemnts,
	// nor copied/moved from another NetAgent
	NetAgent(NetAgent&&) noexcept = delete;
	NetAgent(const NetAgent&) = delete;

	NetAgent& operator=(const NetAgent&) = delete;
	NetAgent& operator=(NetAgent&&) = delete;

	OperatingSystem _connectedOS = OperatingSystem::NA;

	bool IsDisconnectMsg(const PacketBuffer& pConnection) const;

	virtual PacketList ConvertToPackets(const PixelData& data, const PacketType& packetType = PacketType::Invalid);

	// Authenticates connecting computer
	virtual void Handshake(ConnectionPtr& pConnection);

	virtual void Receive(ConnectionPtr& pConnection) = 0;
	virtual void Send(PacketList& packets, ConnectionPtr& pConnection) = 0;

	virtual ~NetAgent() = default;

public:

	void Disconnect(ConnectionPtr& pConnection);
	bool Connected() const;
};