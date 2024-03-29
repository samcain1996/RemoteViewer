#pragma once
#include "Networking/NetAgent.h"

class Client : public NetAgent, public Messageable<PacketPtr> {
private:

	static inline Loggette log = Logger::newStream("Client.log").value();


	string _hostname {};  // Hostname of computer to connect to
	MessageWriter<PacketPtr>*& groupWriter = msgWriter;  // Alias for message queue

	/**
	 * @brief Authenticates connecting computer
	 *
	 * @param pConnection 	The connection
	 */
	void Handshake(ConnectionPtr& pConnection) override;
	vector<thread> threads;

public:
	bool TryConnect(const Ushort = Connection::SERVER_BASE_PORT);

	// Constructors
	Client() = delete;
	Client(const Client&) = delete;
	Client(Client&&) noexcept = delete;

	Client(const string& hostname);
	~Client();

	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;


	/**
	 * @brief Connects to a server via a udp socket
	 *
	 * @param hostname		Name of the computer to connect to
	 * @return true 		Connection succeeded
	 * @return false 		Connection failed
	 */
	bool Connect(const Ushort remotePort, ConnectionPtr& pConnection);

	void Receive(ConnectionPtr& pConnection) override;
	//void Send(PacketList& data, ConnectionPtr& pConnection) override;

	/**
	 * @brief Starts a connection that has already connected
	 *
	 * @param buf 	Buffer holding packet to process
	 * @param size  The amount of data transfered
	 */
	void AdjustForPacketLoss(const PacketBuffer& buf, const int size);
};