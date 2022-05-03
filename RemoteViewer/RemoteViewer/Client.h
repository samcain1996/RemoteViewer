#pragma once
#include "NetAgent.h"
#include <fstream>
#include <thread>

class Client : public NetAgent {
private:
	std::string _hostname;

	ByteArrayMap<PacketPrioQueue> _incompletePackets;

	std::thread _packetWatcherThr;

	std::atomic<bool> _checkPackets;

	void ProcessPacket(const Packet);
	void PacketWatcher();
public:
	Client()				= delete;
	Client(const Client&)	= delete;
	Client(Client&&)		= delete;

	Client(const ushort, const std::string&);

	bool Connect(const std::string&);
	ByteVec AssembleMessage(const PacketGroup&);
	void Receive();
};