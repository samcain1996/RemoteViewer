#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <random>
#include "Networking/Packet.h"
#include "Messages.h"

using boost::asio::ip::tcp;
using std::chrono::steady_clock;

class NetAgent {

protected:
	NetAgent(const std::chrono::seconds& timeout = std::chrono::seconds(30));
	
	// NetAgents shouldn't be instantiated with no arguemnts,
	// nor copied/moved from another NetAgent
	NetAgent() 				  = delete;
	NetAgent(NetAgent&&) 	  = delete;
	NetAgent(const NetAgent&) = delete;

	NetAgent& operator=(const NetAgent&) = delete;
	NetAgent& operator=(NetAgent&&) = delete;

	constexpr const static std::array<Byte, 4> HANDSHAKE_MESSAGE = { 'H', 'I', ':', ')' };
	constexpr const static std::array<Byte, 4> DISCONNECT_MESSAGE = { 'B', 'Y', 'E', '!' };
	
	// Random number generation
	static std::random_device rd;
	static std::mt19937 randomGenerator;

	std::chrono::seconds _timeout;

	boost::asio::io_context _io_context;  // Used for I/O
	// tcp::endpoint _endpoint;

	std::mutex _mutex;
	tcp::socket _socket;
	boost::system::error_code _errcode;

	PacketBuffer _tmpBuffer; // Temporary buffer for receiving/sending packets

	// A map that maps packet groups to a priority queue
	PacketGroupPriorityQueueMap _incompletePackets;

	bool _connected = false;

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	virtual PacketList ConvertToPackets(const ByteVec& data);
	virtual void Handshake() = 0;
	virtual const bool IsDisconnectMsg() const;
	
	virtual void Receive() = 0;
	virtual void Send(const PacketBuffer& data) = 0;
	virtual void ProcessPacket(const Packet& packet) = 0;

public:

	void Disconnect();

	const bool Connected() const;
	
	virtual ~NetAgent();
};