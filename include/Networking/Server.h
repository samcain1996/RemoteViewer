
#pragma once
#include "Networking/NetAgent.h"
#include "QuickShot/Capture.h"

class Server : public NetAgent {

	static inline Loggette log = Logger::newStream("Server.log").value();

private:

	ScreenCapture _screen;

	void SendThread(PacketList&&, ConnectionPtr&);

	void Send(PacketList& packets, ConnectionPtr& pConnection) override;
	void Receive(ConnectionPtr& pConnection) override {};
	void Handshake(ConnectionPtr& pConnection) override;

public:

	Server(const Ushort listenPort);

	Server() = delete;
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	// Serve content to client
	bool Serve();
	void Listen(ConnectionPtr& pConnection);

	~Server();
};
