#include <fstream>
#include "Capture.h"

int main() {

    ScreenCapture screen(2560, 1440, 2560, 1369);

    screen.CaptureScreen();

    // ByteArray capture = nullptr;
    // size_t imgSize = screen.WholeDeal(capture);

    ImageData capture = screen.WholeDeal();

    std::ofstream("img.bmp", std::ios_base::binary).write((char*)(&capture[0]), capture.size());

    return 0;
    
}