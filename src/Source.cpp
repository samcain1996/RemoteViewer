#include <iostream>
#include <fstream>
#include "Application.h"

int main(int argc, char* argv[]) {
#if defined(_WIN32)
	SetProcessDPIAware();
#endif
	SDL_SetMainReady();

	Application::Init();
//	Application::Run();


	return 0;
}