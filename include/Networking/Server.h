
#pragma once
#include "Networking/NetAgent.h"
#include "QuickShot/Capture.h"

class Server : public NetAgent {

private:

	ScreenCapture _screen;

	void Send(PacketList& packets, ConnectionPtr& pConnection) override;
	void Receive(ConnectionPtr& pConnection) override;
	void Handshake(ConnectionPtr& pConnection) override;

public:

	// Servers should only be instantiated with a port number
	Server() = delete;
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	Server(const Ushort listenPort);

	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	// Serve content to client
	bool Serve();
	void Listen(ConnectionPtr& pConnection);

	~Server();
};
