#pragma once
#include "Networking/NetAgent.h"

class Client : public NetAgent, public Messageable<PacketPtr> {
private:
	std::string _hostname{};  // Hostname of computer to connect to

	MessageWriter<PacketPtr>*& groupWriter = msgWriter;

	/**
	 * @brief Processes data from packets and stores them in the
	 * 		  appropriate priority queue.
	 *
	 * @param const Packet		Packet to process
	 *
	 */

	void Handshake(ConnectionPtr&) override;
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

	/**
	 * @brief Receive data from server
	 *
	 */
	void Receive(ConnectionPtr&) override;
	void Start(ConnectionPtr&);
	void Process(const PacketBuffer& buf, int size);

	void Send(PacketList& data, ConnectionPtr&) override;
};