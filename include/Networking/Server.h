#pragma once
#include "Networking/NetAgent.h"
#include "Capture.h"

class Server : public NetAgent, public Messageable<ByteArray> {
private:

	// Converts an arbitrarily long array of bytes
	// into a group of packets
	PacketList ConvertToPackets(ByteArray& bytes, size_t len);

	// Send a buffer of bytes to the client
	bool Send(ByteArray const bytes, const size_t len) override;

	void AsyncSend(ByteArray const bytes, const size_t len) override;
	void AsyncReceive() override;

	void Receive() override {};
	void ProcessPacket(const Packet&) override {};

	ScreenCapture _screen;
	ByteArray _capture = nullptr;

public:

	// Servers should only be instantiated with a port number
	Server() 			  = delete;
	Server(const Server&) = delete;
	Server(Server&&) 	  = delete;
	
	Server(const Ushort listenPort);

	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	void Handshake(bool& connected) override;
	// Serve content to client
	void Serve();

	//MsgWriterPtr<ByteArray> eventWriter = nullptr;

	~Server();
};