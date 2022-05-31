#include <iostream>
#include <fstream>
#include "Application.h"

int main(int argc, char* argv[]) {

#if defined(_WIN32)
	SetProcessDPIAware();
#endif

	SDL_SetMainReady();

	Application::Init(true);
	Application::Run();

	//ScreenCapture screen;
	//screen.CaptureScreen();

	//ByteArray img = nullptr;
	//size_t imgSize = screen.WholeDeal(img);

	//std::ofstream linOut("linuxOut.bmp", std::ios_base::binary);
	//linOut.write((char*)img, imgSize);

	//linOut.close();


	return 0;
}