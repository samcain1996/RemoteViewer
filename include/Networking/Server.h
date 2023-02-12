
#pragma once
#include "Networking/NetAgent.h"
#include "QuickShot/Capture.h"

class Server : public NetAgent {

private:

	ScreenCapture _screen;

	// Send a buffer of bytes to the client
	void Send(PacketList&, int) override;

	void Receive(ConnectionPtr&) override;

	void Handshake(ConnectionPtr&) override;

public:

	// Servers should only be instantiated with a port number
	Server() = delete;
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	Server(const Ushort listenPort, const std::chrono::seconds timeout = std::chrono::seconds(30));

	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	// Serve content to client
	bool Serve();
	void Listen(ConnectionPtr&);

	~Server();
};
