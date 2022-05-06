#include "Capture.h"

Screen::Screen(const size_t srcWidth, const size_t srcHeight, const size_t dstWidth, const size_t dstHeight) {

    // Set the X and Y resolution of the source machine and the destination
    _srcWidth  = srcWidth;
    _srcHeight = srcHeight;

    _dstWidth  = dstWidth;
    _dstHeight = dstHeight;

#if defined(_WIN32)

    // TODO: Move this somewhere else
    SetProcessDPIAware();  // Needed to get the correct resolution in Windows

    _srcHDC = GetDC(GetDesktopWindow());    // Get the device context of the monitor *[1]
    _memHDC = CreateCompatibleDC(_srcHDC);  // Creates a new device context from previous context

    // Create bitmap from the source using the destination's resolution
    _hScreen = CreateCompatibleBitmap(_srcHDC, dstWidth, dstHeight);

    SelectObject(_memHDC, _hScreen);  // Select bitmap into DC *[2]

    // Not likely that source and destination are same resolution.
    // Tell system how to stretch the image
    SetStretchBltMode(_memHDC, HALFTONE);

    _hDIB = NULL;
    _lpbitmap = NULL;

    InitializeBMPHeader();

#elif defined(__APPLE__)

    _context = CGBitmapContextCreate(_currentCapture, _srcWidth, _srcHeight, 
        8, _srcWidth * 4, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

    _bitmapSize = ( (_dstWidth * 32 + 31) / 32) * 4 * _dstHeight; // WHY IS THIS THE FORMULA?

#endif

    _currentCapture  = new Byte[TotalSize()];
    _previousCapture = new Byte[TotalSize()];
    _differenceMap   = new Byte[TotalSize()];
    
    _differences = 0;

}

#if defined(_WIN32)

Screen::Screen() : Screen(GetDeviceCaps(GetDC(NULL), HORZRES), GetDeviceCaps(GetDC(NULL), VERTRES), 1920, 1080) {}

#elif defined(__APPLE__)

Screen::Screen() : Screen(CGDisplayPixelsWide(CGMainDisplayID()), CGDisplayPixelsHigh(CGMainDisplayID()),
900, 1440) {}

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

#endif
    
    delete[] _currentCapture;  
    delete[] _previousCapture; 
    delete[] _differenceMap;
}

void Screen::InitializeBMPHeader() {

    #if defined(_WIN32)
    _bmpInfo.biSize             = sizeof(BITMAPINFOHEADER);
    _bmpInfo.biWidth            = _dstWidth;
    _bmpInfo.biHeight           = _dstHeight;
    _bmpInfo.biPlanes           = 1;
    _bmpInfo.biBitCount         = 32;
    _bmpInfo.biCompression      = BI_RGB;
    _bmpInfo.biSizeImage        = 0;
    _bmpInfo.biXPelsPerMeter    = 0;
    _bmpInfo.biYPelsPerMeter    = 0;
    _bmpInfo.biClrUsed          = 0;
    _bmpInfo.biClrImportant     = 0;

    _bitmapSize = ((_dstWidth * _bmpInfo.biBitCount + 31) / 32) * 4 * _dstHeight; // WHY IS THIS THE FORMULA?

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _lpbitmap = (char*)GlobalLock(_hDIB);

    // Size of the file.
    _bmpHeader.bfSize = _bitmapSize + _bmpInfo.biSize + sizeof(BITMAPFILEHEADER);

    // bfType must always be BM for Bitmaps.
    _bmpHeader.bfType = 0x4D42; // BM.

    // Offset to where the actual bitmap bits start.
    _bmpHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
#endif
}

void Screen::CalculateDifference() {
    _differences = 0;
    for (size_t index = 0; index < TotalSize(); index++) {

        if (_previousCapture[index] != _currentCapture[index]) {
            _differenceMap[_differences++] = _currentCapture[index];
        }
    }
}

void Screen::RecalculateSize() {
#if defined(_WIN32)

    // Resize bitmap
    _bmpInfo.biWidth  = _dstWidth;
    _bmpInfo.biHeight = _dstHeight;

    _bitmapSize = ( (_dstWidth * _bmpInfo.biBitCount + 31) / 32) * 4 * _dstHeight; // WHY IS THIS THE FORMULA?
    _bmpHeader.bfSize = _bitmapSize + _bmpInfo.biSize + sizeof(BITMAPFILEHEADER);

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, _dstWidth, _dstHeight);
    SelectObject(_memHDC, _hScreen);

    // Reallocate the necessary size to store data
    delete[] _currentCapture;
    delete[] _previousCapture;
    delete[] _differenceMap;

    _currentCapture  = new Byte[TotalSize()];
    _previousCapture = new Byte[TotalSize()];
    _differenceMap   = new Byte[TotalSize()];

    // Free _hDIB and re-lock
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _lpbitmap = (char*)GlobalLock(_hDIB);
    #endif
}

const size_t Screen::TotalSize() const {
    return _bitmapSize;
}

void Screen::Resize(const ushort width, const ushort height) {
    _dstWidth  = width;
    _dstHeight = height;
    RecalculateSize();
}

const ByteArray Screen::Bitmap() const {
    return _currentCapture;
}

const size_t Screen::GetDifferences(ByteArray& differenceMap) {
    if (differenceMap) { delete[] differenceMap; }
    differenceMap = new Byte[_differences];
    std::memmove(differenceMap, _differenceMap, _differences);
    return _differences;
}

void Screen::CaptureScreen() {

    std::memmove(_previousCapture, _currentCapture, _bitmapSize);

#if defined(_WIN32)

    StretchBlt(_memHDC, 0, 0, _dstWidth, _dstHeight, _srcHDC, 0, 0, _srcWidth, _srcHeight, SRCCOPY);

    GetObject(_hScreen, sizeof BITMAP, &_screenBMP);

    // Gets the "bits" from the bitmap, and copies them into a buffer 
    // that's pointed to by _lpbitmap.
    GetDIBits(_memHDC, _hScreen, 0,
        (UINT)_screenBMP.bmHeight,
        _lpbitmap,
        (BITMAPINFO*)&_bmpInfo, DIB_RGB_COLORS);

    std::memcpy(_currentCapture, _lpbitmap, _bitmapSize);

#elif defined(__APPLE__)

    _image = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(_context, CGRectMake(0, 0, _srcWidth, _srcWidth), _image);

#endif

    CalculateDifference();

}

const size_t Screen::GetHeader(ByteArray& arr) const {

    #if defined(_WIN32)
    std::memcpy(arr, (LPSTR)&_bmpHeader, sizeof(BITMAPFILEHEADER));
    std::memcpy(&arr[sizeof(BITMAPFILEHEADER)], (LPSTR)&_bmpInfo, sizeof(BITMAPINFOHEADER));

    return sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    #elif defined(__APPLE__)

    // TODO
    return 0;

    #endif
}

