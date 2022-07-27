#include "Capture.h"

ScreenCapture::ScreenCapture(const size_t srcWidth, const size_t srcHeight, const size_t dstWidth, const size_t dstHeight) :
    _srcResolution(srcWidth, srcHeight), _targetResolution(dstWidth, dstHeight) {

#if defined(__linux__)

    _display = XOpenDisplay(nullptr);
    _root = DefaultRootWindow(_display);

    XGetWindowAttributes(_display, _root, &_attributes);

    _srcWidth = _attributes.width;
    _srcHeight = _attributes.height;
#endif

#if defined(_WIN32)

    _srcHDC = GetDC(GetDesktopWindow());  // Get the device context of the monitor [1]
    _memHDC = CreateCompatibleDC(_srcHDC);    // Creates a new device context from previous context

    // Create bitmap from the source using the destination's resolution
    _hScreen = CreateCompatibleBitmap(_srcHDC, _targetResolution.first, _targetResolution.second);

    SelectObject(_memHDC, _hScreen);  // Select bitmap into DC [2]

    // Not likely that source and destination are same resolution.
    // Tell system how to stretch the image
    SetStretchBltMode(_memHDC, HALFTONE);

    _hDIB = NULL;

#endif

    _previousCapture = new Byte[_bitmapSize];

#if defined(__APPLE__)

    _colorspace = CGColorSpaceCreateDeviceRGB();
    _context = CGBitmapContextCreate(_currentCapture, _srcWidth, _srcHeight, 
        8, _srcWidth * 4, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif
    ReInitialize(_targetResolution);
}

#if defined(_WIN32)

ScreenCapture::ScreenCapture() : ScreenCapture(GetDeviceCaps(GetDC(NULL), HORZRES), GetDeviceCaps(GetDC(NULL), VERTRES), 1920, 1080) {}

#elif defined(__APPLE__)

ScreenCapture::ScreenCapture() : ScreenCapture(CGDisplayPixelsWide(CGMainDisplayID()), CGDisplayPixelsHigh(CGMainDisplayID()),
1440, 900) {}

#elif defined(__linux__) 

ScreenCapture::ScreenCapture() : ScreenCapture(1366, 768, 1366, 768) {}

#endif

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

    XDestroyImage(_img);
    XCloseDisplay(_display);

#endif

    if (_previousCapture) {
        delete[](ByteArray)_previousCapture;
    }


}

const BmpFileHeader ScreenCapture::ConstructBMPHeader(const Resolution& targetRes,
        const Ushort bitsPerPixel) {

    std::array<Byte, BMP_HEADER_SIZE> header {};  // TODO: See how to init all vals to 0
    std::memset(header.data(), 0, header.size());
	
	// Dimensions in pixels
    const Ushort width  = targetRes.first;
    const Ushort height = targetRes.second;

    // Size of the data in bytes
    const Uint32 bitmapDataSize = CalulcateBMPFileSize(targetRes, bitsPerPixel);

	// Header components as aliases to header array
    const ByteArray bmpHeader = (ByteArray)&header.data()[0];
    const ByteArray bmpInfo = (bmpHeader + BMP_FILE_HEADER_SIZE);

    bmpHeader[0] = 0x42;
    bmpHeader[1] = 0x4D;

    encode256(&bmpHeader[2],
        width * height * 4 + BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE,
        Endianess::Big);

    bmpHeader[10] = 0x36;

    bmpInfo[0] = 0x28;

    encode256(&bmpInfo[4], width, Endianess::Big);

    encode256(&bmpInfo[8], height, Endianess::Big);

    bmpInfo[12] = 1;

    bmpInfo[14] = bitsPerPixel;
	
    return header;
	
}

constexpr const size_t ScreenCapture::TotalSize() const {
    return _bitmapSize + BMP_HEADER_SIZE;
}

void ScreenCapture::ReInitialize(const Resolution& targetRes) {

    _targetResolution = targetRes;
	
    const Ushort& width = _targetResolution.first;
    const Ushort& height = _targetResolution.second;

    _bitmapSize = CalulcateBMPFileSize(_targetResolution, _bitsPerPixel);

    delete[](ByteArray)_previousCapture;
    _previousCapture = new Byte[_bitmapSize];

#if defined(_WIN32)

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, width, height);
    SelectObject(_memHDC, _hScreen);

    // Free _hDIB and re-lock
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _currentCapture = GlobalLock(_hDIB);

#endif
	
}

const size_t ScreenCapture::WholeDeal(ByteArray& arr) const {

    const size_t captureSize = TotalSize();

    if (arr == nullptr) { arr = new Byte[captureSize]; }

    std::memcpy(arr, &_header, BMP_HEADER_SIZE);

    std::memcpy(&arr[BMP_HEADER_SIZE], _currentCapture, _bitmapSize);

    return captureSize;
}

void ScreenCapture::CaptureScreen() {

	const Ushort& srcWidth  = _srcResolution.first;
    const Ushort& srcHeight = _srcResolution.second;
	
	const Ushort& targetWidth  = _targetResolution.first;
	const Ushort& targetHeight = _targetResolution.second;

    if (_currentCapture)
    std::memcpy(_previousCapture, _currentCapture, _bitmapSize);

#if defined(_WIN32)

    StretchBlt(_memHDC, 0, 0, targetWidth, targetHeight,
        _srcHDC, 0, 0, srcWidth, srcHeight, SRCCOPY);

    GetObject(_hScreen, sizeof BITMAP, &_screenBMP);

    // Should be legal because BITMAPINFO has no padding, all its data members are aligned.
    GetDIBits(_memHDC, _hScreen, 0,
        (UINT)_screenBMP.bmHeight,
        _currentCapture,
        (BITMAPINFO*)(&_header[BMP_FILE_HEADER_SIZE]), DIB_RGB_COLORS);

#elif defined(__APPLE__)

    _image = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(_context, CGRectMake(0, 0, _srcWidth, _srcWidth), _image);
    _currentCapture = _image;

#elif defined(__linux__)

    _img = XGetImage(_display, _root, 0, 0, _srcWidth, _srcHeight, AllPlanes, ZPixmap);
    _currentCapture = _img->data;

#endif

}

