#pragma once
#include "Networking/NetAgent.h"

class Client : public NetAgent, public Messageable<PacketPtr> {
private:
	std::string _hostname{};  // Hostname of computer to connect to

	MessageWriter<PacketPtr>*& groupWriter = msgWriter;

	void Handshake(ConnectionPtr& pConnection) override;

public:

	// Constructors
	Client() = delete;
	Client(const Client&) = delete;
	Client(Client&&) noexcept = delete;

	Client(const std::string& hostname);
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
	const void Connect(const Ushort remotePort, const Action& onConnect);

	void Receive(ConnectionPtr& pConnection) override;
	void Start(ConnectionPtr& pConnection);
	void Process(const PacketBuffer& buf, int size);
	void Send(PacketList& data, ConnectionPtr& pConnection) override;
};