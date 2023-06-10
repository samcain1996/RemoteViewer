
#pragma once
#include "Networking/NetAgent.h"
#include "QuickShot/Capture.h"

struct VideoStream {
	MessageReader<PacketList> reader;
	thread thr;

	VideoStream(const MessageWriter<PacketList>& const writer) : reader(writer) {};
	VideoStream() = delete;
};

class Server : public NetAgent {

	static inline Loggette log = Logger::newStream("Server.log").value();

private:

	ScreenCapture _screen;

	void Handshake(ConnectionPtr& pConnection) override;
	void StreamVideoStream(ConnectionPtr&, MessageReader<PacketList>&);

	vector<MessageWriter<PacketList>*> writers;
	vector<VideoStream*> streams;

public:

	Server(const Ushort listenPort);

	Server() = delete;
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	void Receive(ConnectionPtr& pConnection) override;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	// Serve content to client
	bool Serve();
	bool Listen(ConnectionPtr& pConnection);

	~Server();
};
