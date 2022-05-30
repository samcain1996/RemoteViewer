#include "NetAgent.h"

std::random_device NetAgent::rd{};

std::mt19937 NetAgent::randomGenerator(rd());

NetAgent::NetAgent(const Ushort localPort) : _localPort(localPort), _localEndpoint(udp::v4(), _localPort),
	_socket(_io_context, _localEndpoint), _keepAlive(true) {}

NetAgent::~NetAgent()
{
}
