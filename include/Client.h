#pragma once
#include "NetAgent.h"
#include "ClientWindow.h"
#include "Capture.h"

class Client : public NetAgent {
private:
	std::string _hostname;  // Hostname of computer to connect to

	// A map that maps packet groups to a priority queue
	PacketGroupPriorityQueueMap _incompletePackets;

	ApplicationMessageWriter _msgWriter;

	ClientWindow _window;

	// Thread to assemble data from packets
	std::thread _packetWatcherThr;

	// Thread to display remote screen
	std::thread _windowThr;

	// Thread-safe flag to function as flow control for thread
	std::atomic<bool> _checkPackets;

	/**
	 * @brief Processes data from packets and stores them in the
	 * 		  appropriate priority queue.
	 * 
	 * @param const Packet		Packet to process
	 * 
	 */
	void ProcessPacket(const Packet);

	/**
	 * @brief Keep track of incoming packets and determine if 
	 *        all packets for a given message have arrived
	 * 
	 */
	void PacketWatcher();

	void ScreenDisp();
public:

	// Constructors
	Client()				= delete;
	Client(const Client&)	= delete;
	Client(Client&&)		= delete;

	Client(const ushort, const std::string&);

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