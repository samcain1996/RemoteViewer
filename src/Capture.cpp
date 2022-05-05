#include "Capture.h"

Screen::Screen(const ushort srcWidth, const ushort srcHeight, const ushort dstWidth, const ushort dstHeight) {

    _srcWidth  = srcWidth;
    _srcHeight = srcHeight;

    _dstWidth  = dstWidth;
    _dstHeight = dstHeight;

#if defined(_WIN32)
    SetProcessDPIAware();

    _srcHDC = GetDC(GetDesktopWindow());
    _memHDC = CreateCompatibleDC(_srcHDC);

    _hScreen = CreateCompatibleBitmap(_srcHDC, dstWidth, dstHeight);

    SelectObject(_memHDC, _hScreen);

    SetStretchBltMode(_memHDC, HALFTONE);

    _hDIB = NULL;
    _lpbitmap = NULL;

    InitializeBMPHeader();

    _currentCapture  = new Byte[TotalSize()];
    _previousCapture = new Byte[TotalSize()];
    _differenceMap   = new Byte[TotalSize()];

#elif defined(_APPLE_)

    context = CGBitmapContextCreate(_currentCapture, srcWidth, srcHeight, 
        8, width * 4, colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif
    _differences = 0;
}

#if defined(_WIN32)

Screen::Screen() : Screen(GetDeviceCaps(GetDC(NULL), HORZRES), GetDeviceCaps(GetDC(NULL), VERTRES), 1920, 1080) {}

#elif defined(_APPLE_)

Screen::Screen() : Screen(CGDisplayPixelWide(CGMainDisplayID()), CGDisplayPixelHigh(CGMainDisplayID())) {}

#endif

Screen::~Screen() {

#if defined(_WIN32)

    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    DeleteObject(_hScreen);

    ReleaseDC(NULL, _srcHDC);
    DeleteObject(_memHDC);

#elif

    CGImageRelease(_image);
    CGContextRelease(_context);
    CGColorSpaceRelease(_colorspace);

#endif
    
    delete[] _currentCapture;  
    delete[] _previousCapture; 
}

void Screen::InitializeBMPHeader() {
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
    _bmpInfo.biWidth  = _dstWidth;
    _bmpInfo.biHeight = _dstHeight;

    _bitmapSize = ( (_dstWidth * _bmpInfo.biBitCount + 31) / 32) * 4 * _dstHeight; // WHY IS THIS THE FORMULA?
    _bmpHeader.bfSize = _bitmapSize + _bmpInfo.biSize + sizeof(BITMAPFILEHEADER);

    DeleteObject(_hScreen);

    _hScreen = CreateCompatibleBitmap(_srcHDC, _dstWidth, _dstHeight);
    SelectObject(_memHDC, _hScreen);

    delete[] _currentCapture;
    delete[] _previousCapture;
    delete[] _differenceMap;

    _currentCapture  = new Byte[TotalSize()];
    _previousCapture = new Byte[TotalSize()];
    _differenceMap   = new Byte[TotalSize()];

    // Free _hDIB and recalculate
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _lpbitmap = (char*)GlobalLock(_hDIB);
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

const size_t Screen::GetDifferences(ByteArray differenceMap) {
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

#elif defined(_APPLE_)

    image = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(context, CGRectMake(0, 0, _srcWidth, _srcWidth), image);

#endif

    CalculateDifference();

}

void Screen::GetHeader(ByteArray arr) {
    std::memcpy(arr, (LPSTR)&_bmpHeader, sizeof(BITMAPFILEHEADER));
    std::memcpy(&arr[sizeof(BITMAPFILEHEADER)], (LPSTR)&_bmpInfo, sizeof(BITMAPINFOHEADER));
}

