#include <fstream>
#include "Capture.h"

int main() {

    ScreenCapture screen(1920, 1080, 1920, 1080);

    screen.CaptureScreen();

    ByteArray capture = nullptr;
    size_t imgSize = screen.WholeDeal(capture);

    std::ofstream("img.bmp", std::ios_base::binary).write((char*)capture, imgSize);

    return 0;
    
}