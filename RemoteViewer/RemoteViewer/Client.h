#pragma once
#include "NetAgent.h"

class Client : public NetAgent {
private:
	std::string m_hostname;

	ByteArrayMap<PacketPrioQueue> m_incompletePackets;
	ByteArrayMap<size_t> m_packetGroupMsgSizes;

	std::thread packetWatcherThr;

	std::atomic<bool> m_checkPackets;

	void ProcessPacket(const Packet);
	void PacketWatcher();
public:
	Client() = delete;
	Client(const Client&&) = delete;

	Client(const ushort, const std::string&);

	bool Connect(const std::string&);
	std::vector<Byte> AssembleMessage(const PacketGroup&);
	void Receive();
};