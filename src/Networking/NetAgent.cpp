#include "Networking/NetAgent.h"

std::random_device NetAgent::rd{};

std::mt19937 NetAgent::randomGenerator(rd());

NetAgent::NetAgent(const Ushort localPort) : _localPort(localPort), _localEndpoint(udp::v4(), _localPort),
	_socket(_io_context, _localEndpoint){}

NetAgent::~NetAgent()
{
}

const bool NetAgent::IsDisconnectMsg() const {

	return std::memcmp(_tmpBuffer.data(), DISCONNECT_MESSAGE, DISCONNECT_SIZE) == 0;

}
