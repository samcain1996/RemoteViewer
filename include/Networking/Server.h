#pragma once
#include "Networking/NetAgent.h"
#include "Capture.h"

class Server : public NetAgent {
private:

	ScreenCapture _screen;
	tcp::acceptor _acceptor;
	Ushort _localport;

	// Send a buffer of bytes to the client
	void Send(const PacketBuffer& data) override;
	void NewSend(Byte* data, size_t size);

	void Receive() override;
	// void ProcessPacket(const Packet&) override {};

	void Handshake(bool& isWindows) override;

public:

	// Servers should only be instantiated with a port number
	Server() 			  = delete;
	Server(const Server&) = delete;
	Server(Server&&) 	  = delete;
	
	Server(const Ushort listenPort, const std::chrono::seconds timeout = std::chrono::seconds(30));

	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	// Serve content to client
	bool Serve();
	void Listen();

	~Server();
};