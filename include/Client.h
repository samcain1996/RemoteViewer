#pragma once
#include "NetAgent.h"
#include "RenderWindow.h"

class Client : public NetAgent {

	using GroupReadyReader = MessageReader<PacketGroup>;
	using GroupReadyWriter = MessageWriter<PacketGroup>;

private:
	std::string _hostname;  // Hostname of computer to connect to

	// A map that maps packet groups to a priority queue
	PacketGroupPriorityQueueMap _incompletePackets;
	std::unordered_map<Uint32, Uint32> _packetGroups;

	GroupReadyWriter _msgWriter;  // Used to send messages to _window

	RenderWindow* _window;  // Object to handle window rendering

	// Thread to display remote screen
	std::thread _windowThr;

	/**
	 * @brief Processes data from packets and stores them in the
	 * 		  appropriate priority queue.
	 * 
	 * @param const Packet		Packet to process
	 * 
	 */
	void ProcessPacket(const Packet);

public:

	// Constructors
	Client()				= delete;
	Client(const Client&)	= delete;
	Client(Client&&)		= delete;

	Client(const Ushort, const std::string&);

	~Client();

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
	void Receive();
};