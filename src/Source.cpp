#include <iostream>
#include "Server.h"
#include "Client.h"
#include "Capture.h"

void x(int port, const std::string hostname) {
	Client client(port, hostname);

	client.Connect("10008");
}

void y(int port) {
	Server server(port);

	server.Listen();
}

int main() {

	//std::thread server(y, 10008);
	//std::thread client(x, 10009, "192.168.50.160");

	//server.join();
	//client.join();

	// Client client(10009, "192.168.50.160");
	// client.Connect("30027"); 

	Screen screen(1440, 900, 1440, 900);
	std::ofstream out("headerTest.bmp", std::ios_base::binary);
	ByteArray byte = new Byte[BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE];
	screen.GetHeader(byte);
	out.write((char*)byte, BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE);
	//std::ifstream in("capture2.dat", std::ios_base::binary);
	//ByteArray arr = new Byte[5184000];
	//in.read((char*)arr,5184000);
	//out.write((char*)arr, 5184000);
	screen.CaptureScreen();

	//Byte* header = new Byte[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
	//screen.GetHeader(header);

	Byte* buf = new Byte[screen.TotalSize()];

	memcpy(buf, screen.Bitmap(), screen.TotalSize());

	//std::ofstream capture2("capture2.bmp", std::ios_base::binary);
	//capture2.write((char*)header, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	out.write((char*)buf, screen.TotalSize());
	//capture2.close();

	return 0;
}