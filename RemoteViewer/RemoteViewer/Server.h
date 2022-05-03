#pragma once
#include "NetAgent.h"
#include <random>

class Server : public NetAgent {
private:
	std::random_device rd;
	std::mt19937 randomGenerator;
	// Converts an arbitrarily long array of bytes
	// into a group of packets
	PacketList ConvertToPackets(std::vector<Byte>&);
public:
	Server() = delete;
	Server(const Server&&) = delete;
	Server(const unsigned short listenPort);

	// Listen for connections
	void Listen();

	// Serve content to client
	void Serve();

	// Send array of bytes to Client
	void Send(std::vector<Byte>&);
};