//#include <iostream>
//#include "Server.h"
//#include "Client.h"
//
//void x(int port, const std::string hostname) {
//	Client client(port, hostname);
//
//	client.Connect("10008");
//}
//
//void y(int port) {
//	Server server(port);
//
//	server.Listen();
//}

#include "Capture.h"
#include <fstream>

int main() {

	//std::thread server(y, 10008);
	//std::thread client(x, 10009, "192.168.50.72");

	//server.join();
	//client.join();

	Screen screen;
	screen.CaptureScreen();

	Byte* header = new Byte[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
	screen.GetHeader(header);

	Byte* buf = new Byte[screen.TotalSize()];

	std::memcpy(buf, screen.Bitmap(), screen.TotalSize());

	std::ofstream capture1("capture1.bmp", std::ios_base::binary);
	capture1.write((char*)buf, screen.TotalSize());
	capture1.close();

	std::ofstream capture2("capture2.bmp", std::ios_base::binary);
	capture2.write((char*)header, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	capture2.write((char*)buf, screen.TotalSize());
	capture2.close();

	return 0;
}