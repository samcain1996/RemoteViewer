#pragma once
#include "NetAgent.h"
#include "Capture.h"

class Server : public NetAgent {
private:

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	PacketList ConvertToPackets(ByteArray& bytes, size_t len);

	// Send a buffer of bytes to the client
	void Send(ByteArray const bytes, const size_t len) override;

	void Receive() override {};
	void ProcessPacket(const Packet&) override {};

	ScreenCapture _screen;
	ByteArray _capture = nullptr;

public:

	// Severs should only be instantiated with a port number
	Server() 			  = delete;
	Server(const Server&) = delete;
	Server(Server&&) 	  = delete;
	
	Server(const Ushort listenPort);

	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	// Listen for connections
	void Listen();
	// Serve content to client
	void Serve();

	MsgWriterPtr<ByteArray> eventWriter = nullptr;

	~Server();
};