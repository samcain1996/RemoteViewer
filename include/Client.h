#pragma once
#include "NetAgent.h"
#include "RenderWindow.h"

class Client : public NetAgent {
private:
	std::string _hostname;  // Hostname of computer to connect to


	/**
	 * @brief Processes data from packets and stores them in the
	 * 		  appropriate priority queue.
	 * 
	 * @param const Packet		Packet to process
	 * 
	 */
	void ProcessPacket(const Packet&) override;

	void Send(ByteArray bytes, size_t len) override {};

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
	bool Connect(const std::string&);

	///**
	// * @brief Assembles complete message from individual packets
	// * 
	// * @param const PacketGroup& 	Packet group to assemble
	// * @return ByteVec 				Vector of bytes representing the message
	// */
	//size_t AssembleMessage(const PacketGroup&, ByteArray&);

	/**
	 * @brief Receive data from server
	 * 
	 */
	void Receive() override;


	MessageWriter<PacketGroup>* writer = nullptr;
};