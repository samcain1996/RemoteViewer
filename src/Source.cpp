#include <iostream>
#include "Server.h"
#include "Client.h"
//#include "Capture.h"

void x(int port, const std::string hostname) {
	Client client(port, hostname);

	client.Connect("10008");
}

void y(int port) {
	Server server(port);

	server.Listen();
}

int main(int argc, char* argv[]) {

#if defined(_WIN32)
	SetProcessDPIAware();
#endif

	SDL_SetMainReady();
	SDL_Init(SDL_INIT_EVERYTHING);
	std::thread clientThr(x, 10009, "192.168.50.160");
	std::thread serverThr(y, 10008);

	serverThr.join();
	clientThr.join();

	SDL_Quit();

	return 0;
}