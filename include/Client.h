#pragma once
#include "NetAgent.h"

class Client : public NetAgent, public Messageable<PacketPriorityQueue*>, public Messageable<SDL_Event> {
private:
	std::string _hostname;  // Hostname of computer to connect to

	PacketGroupMap _packetGroups;

	MessageWriter<PacketPriorityQueue*>*& groupWriter = Messageable<PacketPriorityQueue*>::msgWriter;
	MessageReader<SDL_Event>*& eventReader = Messageable<SDL_Event>::msgReader;

	/**
	 * @brief Processes data from packets and stores them in the
	 * 		  appropriate priority queue.
	 * 
	 * @param const Packet		Packet to process
	 * 
	 */
	void ProcessPacket(const Packet&) override;

	void Send(ByteArray const bytes, const size_t len) override;

public:

	// Constructors
	Client()				= delete;
	Client(const Client&)	= delete;
	Client(Client&&)		= delete;

	Client(const Ushort, const std::string&);

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
	bool Connect(const std::string& hostname);

	/**
	 * @brief Receive data from server
	 * 
	 */
	void Receive() override;
};