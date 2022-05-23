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
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_EVERYTHING);

	std::thread clientThr(x, 10009, "192.168.50.160");
	std::thread serverThr(y, 10008);

	serverThr.join();
	clientThr.join();

	SDL_Quit();
	
	//std::ifstream in("capture2.dat", std::ios_base::binary);
	//ByteArray arr = new Byte[5184000];
	//in.read((char*)arr,5184000);
	//out.write((char*)arr, 5184000);

	//Byte* header = new Byte[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
	//screen.GetHeader(header);

	

	

	//std::ofstream capture2("capture2.bmp", std::ios_base::binary);
	//capture2.write((char*)header, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	//capture2.close();

	return 0;
}