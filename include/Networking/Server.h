#pragma once
#include "Networking/NetAgent.h"
#include "Capture.h"

class Server : public NetAgent {
	friend class ServerInitWindow;
private:

	std::chrono::seconds _timeout;

	// Send a buffer of bytes to the client
	bool Send(const ByteVec& data) override;

	void Receive() override {};
	void ProcessPacket(const Packet&) override {};

	void Handshake() override;

	ScreenCapture _screen;

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
	bool Listen();

	~Server();
};