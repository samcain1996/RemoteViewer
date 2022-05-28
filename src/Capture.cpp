#include "Capture.h"

Screen::Screen(const size_t srcWidth, const size_t srcHeight, const size_t dstWidth, const size_t dstHeight) :
    _srcWidth(srcWidth), _srcHeight(srcHeight), _dstWidth(dstWidth), _dstHeight(dstHeight) {

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
    _hScreen = CreateCompatibleBitmap(_srcHDC, _dstWidth, _dstHeight);

    SelectObject(_memHDC, _hScreen);  // Select bitmap into DC [2]

    // Not likely that source and destination are same resolution.
    // Tell system how to stretch the image
    SetStretchBltMode(_memHDC, HALFTONE);

    _hDIB = NULL;
    _lpbitmap = NULL;

#endif

    InitializeBMPHeader();

    _previousCapture = new Byte[_bitmapSize];

#if defined(__APPLE__)

    _colorspace = CGColorSpaceCreateDeviceRGB();
    _context = CGBitmapContextCreate(_currentCapture, _srcWidth, _srcHeight, 
        8, _srcWidth * 4, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif

}

#if defined(_WIN32)

Screen::Screen() : Screen(GetDeviceCaps(GetDC(NULL), HORZRES), GetDeviceCaps(GetDC(NULL), VERTRES), 1920, 1080) {}

#elif defined(__APPLE__)

Screen::Screen() : Screen(CGDisplayPixelsWide(CGMainDisplayID()), CGDisplayPixelsHigh(CGMainDisplayID()),
1440, 900) {}

#elif defined(__linux__) 

Screen::Screen() : Screen(1366, 768, 1366, 768) {}

#endif

Screen::~Screen() {

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
    
}

void Screen::InitializeBMPHeader() {

    _bitmapSize = ((_dstWidth * _bitsPerPixel + 31) / 32) * 4 * _dstHeight; // WHY IS THIS THE FORMULA?

    ByteArray _bmpHeader = (ByteArray)&_header[0];
    ByteArray _bmpInfo = (_bmpHeader + BMP_FILE_HEADER_SIZE);

    std::memset(_bmpHeader, 0x00, BMP_HEADER_SIZE);

    _bmpHeader[0] = 0x42;
    _bmpHeader[1] = 0x4D;

    //Byte temp[4];
    encode256(&_bmpHeader[2],
        _srcWidth * _srcHeight * 4 + BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE,
        Endianess::Big);

    //std::memmove(&_bmpHeader[2], temp, 4);

    _bmpHeader[10] = 0x36;

    _bmpInfo[0] = 0x28;

    encode256(&_bmpInfo[4], _srcWidth, Endianess::Big);
    //std::memmove(&_bmpInfo[4], temp, 4);

    encode256(&_bmpInfo[8], _srcHeight, Endianess::Big);
    //std::memmove(&_bmpInfo[8], temp, 4);

    _bmpInfo[12] = 1;

    _bmpInfo[14] = _bitsPerPixel;

    #if defined(_WIN32)

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _currentCapture = GlobalLock(_hDIB);

#endif
}

void Screen::RecalculateSize() {

    _bitmapSize = ((_dstWidth * _bitsPerPixel + 31) / 32) * 4 * _dstHeight; // WHY IS THIS THE FORMULA?

    delete[] _previousCapture;
    _previousCapture = new Byte[_bitmapSize];

#if defined(_WIN32)

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, _dstWidth, _dstHeight);
    SelectObject(_memHDC, _hScreen);

   // _differenceMap   = new Byte[_bitmapSize];

    // Free _hDIB and re-lock
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _currentCapture = GlobalLock(_hDIB);

    #endif

    InitializeBMPHeader();
}

const size_t Screen::TotalSize() const {
    return _bitmapSize + BMP_HEADER_SIZE;
}

void Screen::Resize(const Ushort width, const Ushort height) {
    _dstWidth  = width;
    _dstHeight = height;
    RecalculateSize();
}

const size_t Screen::WholeDeal(ByteArray& arr) const {

    if (arr == nullptr) { arr = new Byte[_bitmapSize + BMP_HEADER_SIZE]; }

    std::memcpy(arr, &_header, BMP_HEADER_SIZE);

    std::memcpy(&arr[BMP_HEADER_SIZE], _currentCapture, _bitmapSize);

    return _bitmapSize + BMP_HEADER_SIZE;
}

void Screen::CaptureScreen() {

    if (_currentCapture)
    std::memcpy(_previousCapture, _currentCapture, _bitmapSize);

#if defined(_WIN32)

    StretchBlt(_memHDC, 0, 0, _dstWidth, _dstHeight, _srcHDC, 0, 0, _srcWidth, _srcHeight, SRCCOPY);

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

    img = XGetImage(display, root, 0, 0, _srcWidth, _srcHeight, AllPlanes, ZPixmap);
    _currentCapture = img->data;

#endif

    //CalculateDifference();

}

