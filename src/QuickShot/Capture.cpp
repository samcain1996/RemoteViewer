#include "QuickShot/Capture.h"

Resolution ScreenCapture::DefaultResolution = RES_720;// ScreenCapture::NativeResolution();

Resolution ScreenCapture::NativeResolution(const bool reinit) {

    static auto retrieveRes = []() {

#if defined(_WIN32)

        SetProcessDPIAware();
        return Resolution{ GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };

#elif defined(__linux__)

        XGetWindowAttributes(_display, _root, &_attributes);
        return Resolution{ _attributes.width, _attributes.height };

#elif defined(__APPLE__)

        const auto mainDisplayId = CGMainDisplayID();
        return Resolution{ (int)CGDisplayPixelsWide(mainDisplayId), (int)CGDisplayPixelsHigh(mainDisplayId) };

#endif

    };

    static Resolution NATIVE_RESOLUTION = retrieveRes();

    if (reinit) { NATIVE_RESOLUTION = retrieveRes(); }

    return NATIVE_RESOLUTION;

}

ScreenCapture::ScreenCapture(const int width, const int height) {

    _resolution.width = width;
    _resolution.height = height;

    // Capture the entire screen by default	
    _captureArea.right = NativeResolution().width;
    _captureArea.bottom = NativeResolution().height;

#if defined(_WIN32)

    _srcHDC = GetDC(GetDesktopWindow());      // Get the device context of the monitor [1]
    _memHDC = CreateCompatibleDC(_srcHDC);    // Creates a new device context from previous context

    SetStretchBltMode(_memHDC, HALFTONE);     // Set the stretching mode to halftone

    _hDIB = NULL;

#elif defined(__APPLE__)

    _colorspace = CGColorSpaceCreateDeviceRGB();

#endif

    Resize(_resolution);

}

ScreenCapture::ScreenCapture(const Resolution& res, const ScreenArea& areaToCapture) :
    ScreenCapture(res.width, res.height) {
    _captureArea = std::min(areaToCapture, static_cast<ScreenArea>(ScreenCapture::NativeResolution()));
}

ScreenCapture::~ScreenCapture() {

#if defined(_WIN32)

    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    DeleteObject(_hScreen);

    ReleaseDC(NULL, _srcHDC);
    DeleteObject(_memHDC);

#elif defined(__APPLE__)

    CGImageRelease(_image);
    CGContextRelease(_context);
    CGColorSpaceRelease(_colorspace);

#elif defined(__linux__)

    XDestroyImage(_image);

#endif

}


ScreenCapture::ScreenCapture(const ScreenCapture& other) : ScreenCapture(other._resolution) {}

const Resolution& ScreenCapture::GetResolution() const { return _resolution; }

void ScreenCapture::Resize(const Resolution& resolution) {

    _resolution = resolution;

    _captureSize = CalculateBMPFileSize(_resolution, _bitsPerPixel, false);
    _header = ConstructBMPHeader(_resolution, _bitsPerPixel);

    _pixelData = PixelData(_captureSize, '\0');

#if defined(_WIN32)

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, _resolution.width, _resolution.height);

    SelectObject(_memHDC, _hScreen);  // Select bitmap into DC [2]

    // Free _hDIB and re-lock
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _captureSize);
    (MyByte*)GlobalLock(_hDIB);

#elif defined(__APPLE__)

    // Ok to call on null objects
    CGImageRelease(_image);
    CGContextRelease(_context);

    _context = CGBitmapContextCreate(_pixelData.data(), _resolution.width, _resolution.height,
        BITS_PER_CHANNEL, _resolution.width * NUM_COLOR_CHANNELS, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif

}

void ScreenCapture::Crop(const ScreenArea& area) { _captureArea = std::min(area, static_cast<ScreenArea>(ScreenCapture::NativeResolution())); }

const PixelData ScreenCapture::WholeDeal() const {

    PixelData wholeDeal(_header.begin(), _header.end());
    std::copy(_pixelData.begin(), _pixelData.end(), std::back_inserter(wholeDeal));

    return wholeDeal;
}

const PixelData& ScreenCapture::CaptureScreen() {

    const Resolution& captureAreaRes = static_cast<Resolution>(_captureArea);

#if defined(_WIN32)

    // Resize to target resolution
    StretchBlt(_memHDC, 0, 0, _resolution.width, _resolution.height,
        _srcHDC, _captureArea.left, _captureArea.top, _captureArea.right, _captureArea.bottom, SRCCOPY);

    GetObject(_hScreen, sizeof(_screenBMP), &_screenBMP);

    // Store screen data in _pixelData
    // Should be legal because BITMAPINFO has no padding, all its data members are aligned.
    GetDIBits(_memHDC, _hScreen, 0,
        (UINT)_screenBMP.bmHeight,
        _pixelData.data(),
        (BITMAPINFO*)(&_header[BMP_FILE_HEADER_SIZE]), DIB_RGB_COLORS);

#elif defined(__APPLE__)

    _image = CGDisplayCreateImageForRect(CGMainDisplayID(),
        CGRectMake(_captureArea.left, _captureArea.top, captureAreaRes.width, captureAreaRes.height));
    CGContextDrawImage(_context, CGRectMake(0, 0,
        _resolution.width, _resolution.height), _image);

#elif defined(__linux__)

    _image = XGetImage(_display, _root, _captureArea.left, _captureArea.top,
        captureAreaRes.width, captureAreaRes.height, AllPlanes, ZPixmap);

    _pixelData = PixelData(_image->data, _image->data + CalculateBMPFileSize(captureAreaRes, 32, false));
    _pixelData = Scaler::Scale(_pixelData, captureAreaRes, _resolution);

#endif

    return _pixelData;
}

void ScreenCapture::SaveToFile(const PixelData& imageAndHeader, std::string filename) {
    // Add file extension if not present
    if (filename.find(".bmp") == std::string::npos) {
        filename += ".bmp";
    }

    std::ofstream(filename, std::ios::binary).write((char*)imageAndHeader.data(), imageAndHeader.size());
}

void ScreenCapture::SaveToFile(const PixelData& image, const BmpFileHeader& header, std::string filename) {
    if (filename.find(".bmp") == std::string::npos) {
        filename += ".bmp";
    }

    std::ofstream outputFile(filename, std::ios::binary);
    outputFile.write((char*)header.data(), header.size());
    outputFile.write((char*)image.data(), image.size());
}

void ScreenCapture::SaveToFile(const PixelData& image, const Resolution& resolution, std::string filename) {
    SaveToFile(image, ConstructBMPHeader(resolution), filename);
}

void ScreenCapture::SaveToFile(const std::string& filename) const {
    SaveToFile(_pixelData, _resolution, filename);
}