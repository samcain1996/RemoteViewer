#include <fstream>
#include "Capture.h"

int main() {

    ScreenCapture screen(2560, 1440, 2560, 1369);

    screen.CaptureScreen();

    //  ByteArray capture = nullptr;
    //  size_t imgSize = screen.GetImageData(capture);

    ImageData capture = screen.GetImageData();
    BmpFileHeader header = screen.ConstructBMPHeader(Resolution(2560, -1369));

    std::ofstream output("imgNew.bmp", std::ios_base::binary);
    output.write((char*)header.data(), header.size());
    output.write((char*)capture.data(), capture.size());
    

    return 0;
    
}