#pragma once
#include "NetAgent.h"
#include <random>
#include "Capture.h"

class Server : public NetAgent {
private:
	std::random_device rd;  // Used to seed random number generator

	// Random number generator, C-style rand does not have enough precision
	std::mt19937 randomGenerator;

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	PacketList ConvertToPackets(ByteVec&);
public:

	// Severs should only be instantiated with a port number
	Server() = delete;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	
	Server(const unsigned short listenPort);

	// Listen for connections
	void Listen();

	// Serve content to client
	void Serve();

	// Send a buffer of bytes to the client
	void Send(ByteVec&);
};