#include <iostream>
#include "Server.h"
#include "Client.h"

void x(int port, const std::string hostname) {
	Client client(port, hostname);

	client.Connect("10005");
}

void y(int port) {
	Server server(port);

	server.Listen();
}

int main() {

	std::thread server(y, 10005);
	std::thread client(x, 10004, "192.168.50.160");

	server.join();
	client.join();

	return 0;
}