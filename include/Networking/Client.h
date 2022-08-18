#pragma once
#include "Networking/NetAgent.h"

class Client : public NetAgent, public Messageable<PacketPriorityQueue*> {
private:
	std::string _hostname;  // Hostname of computer to connect to

	PacketGroupMap _packetGroups;

	int _remotePort = -1;

	MessageWriter<PacketPriorityQueue*>*& groupWriter = Messageable<PacketPriorityQueue*>::msgWriter;

	/**
	 * @brief Processes data from packets and stores them in the
	 * 		  appropriate priority queue.
	 * 
	 * @param const Packet		Packet to process
	 * 
	 */
	void ProcessPacket(const Packet& packet) override;

public:

	// Constructors
	Client()				= delete;
	Client(const Client&)   = delete;
	Client(Client&&)		= delete;

	Client(const Ushort port, const std::string& hostname);

	~Client();

	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	/**
	 * @brief Connects to a server via a udp socket
	 * 
	 * @param hostname		Name of the computer to connect to
	 * @return true 		Connection succeeded
	 * @return false 		Connection failed
	 */
	bool Connect(const std::string& serverPort);

	void Handshake() override;

	/**
	 * @brief Receive data from server
	 * 
	 */
	void Receive() override;
};