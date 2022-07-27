#include <fstream>
#include "Capture.h"

int main() {

    ScreenCapture screen(2560, 1600, 2560, 1600);

    screen.CaptureScreen();

    ByteArray capture = nullptr;
    size_t imgSize = screen.GetImageData(capture);

    std::ofstream("img.dat", std::ios_base::binary).write((char*)capture, imgSize);

    return 0;
    
}