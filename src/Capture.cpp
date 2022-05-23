#include "Capture.h"

Screen::Screen(const size_t srcWidth, const size_t srcHeight, const size_t dstWidth, const size_t dstHeight) :
    _srcWidth(srcWidth), _srcHeight(srcHeight), _dstWidth(dstWidth), _dstHeight(dstHeight),
    _bitmapSize(((dstWidth * 32 + 31) / 32) * 4 * dstHeight) {

    _currentCapture  = new Byte[_bitmapSize];
    _previousCapture = new Byte[_bitmapSize];

#if defined(_WIN32)

    // TODO: Move this somewhere else
    SetProcessDPIAware();  // Needed to get the correct resolution in Windows

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

    InitializeBMPHeader();

#elif defined(__APPLE__)

    _colorspace = CGColorSpaceCreateDeviceRGB();

    std::memset(_bitmapHeader, 0x00, BMP_FILE_HEADER_SIZE);
    
    _bitmapHeader[0] = 0x42;
    _bitmapHeader[1] = 0x4D;
    
    Byte temp[4];
    encode256(temp, 
        _srcWidth * _srcHeight * 4 + BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE,
        Endianess::Little);

    std::memmove(&_bitmapHeader[2], temp, 4);

    _bitmapHeader[10] = 0x36;

    std::memset(_bitmapInfo, 0x00, BMP_INFO_HEADER_SIZE);

    _bitmapInfo[0] = 0x28;

    encode256(temp, _srcWidth, Endianess::Little);
    std::memmove(&_bitmapInfo[4], temp, 4);

    encode256(temp, _srcHeight, Endianess::Little);
    std::memmove(&_bitmapInfo[8], temp, 4);

    _bitmapInfo[9] = 1;

    _bitmapInfo[11] = 4;

    _context = CGBitmapContextCreate(_currentCapture, _srcWidth, _srcHeight, 
        8, _srcWidth * 4, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif

}

#if defined(_WIN32)

Screen::Screen() : Screen(GetDeviceCaps(GetDC(NULL), HORZRES), GetDeviceCaps(GetDC(NULL), VERTRES), 1920, 1080) {}

#elif defined(__APPLE__)

Screen::Screen() : Screen(CGDisplayPixelsWide(CGMainDisplayID()), CGDisplayPixelsHigh(CGMainDisplayID()),
1440, 900) {}

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
    //delete[] _differenceMap;
}

void Screen::InitializeBMPHeader() {

    #if defined(_WIN32)
    _bmpInfo.biSize             = BMP_INFO_HEADER_SIZE;
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
    _bmpHeader.bfSize = _bitmapSize + _bmpInfo.biSize + BMP_FILE_HEADER_SIZE;

    // bfType must always be BM for Bitmaps.
    _bmpHeader.bfType = 0x4D42; // BM.

    // Offset to where the actual bitmap bits start.
    _bmpHeader.bfOffBits = (DWORD)BMP_FILE_HEADER_SIZE + (DWORD)BMP_INFO_HEADER_SIZE;
#endif
}

void Screen::CalculateDifference() {

    //_differenceArray.Clear();
    //
    //for (size_t index = 0; index < _bitmapSize; index++) {

    //    if (_previousCapture[index] != _currentCapture[index]) {
    //        Uint32 beginIdx = index;

    //        for (++index; index < _bitmapSize && _previousCapture[index] != _currentCapture[index]; index++) {};

    //        _differenceArray.AddDifference(beginIdx, index, &_currentCapture[beginIdx]);

    //    }
    //}
}

void Screen::RecalculateSize() {
#if defined(_WIN32)

    // Resize bitmap
    _bmpInfo.biWidth  = _dstWidth;
    _bmpInfo.biHeight = _dstHeight;

    _bitmapSize = ( (_dstWidth * _bmpInfo.biBitCount + 31) / 32) * 4 * _dstHeight; // WHY IS THIS THE FORMULA?
    _bmpHeader.bfSize = _bitmapSize + _bmpInfo.biSize + BMP_FILE_HEADER_SIZE;

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, _dstWidth, _dstHeight);
    SelectObject(_memHDC, _hScreen);

    // Reallocate the necessary size to store data
    delete[] _currentCapture;
    delete[] _previousCapture;
    // delete[] _differenceMap;

    _currentCapture  = new Byte[_bitmapSize];
    _previousCapture = new Byte[_bitmapSize];
   // _differenceMap   = new Byte[_bitmapSize];

    // Free _hDIB and re-lock
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _lpbitmap = (char*)GlobalLock(_hDIB);
    #endif
}

const size_t Screen::TotalSize() const {
    return _bitmapSize + BMP_HEADER_SIZE;
}

void Screen::Resize(const Ushort width, const Ushort height) {
    _dstWidth  = width;
    _dstHeight = height;
    RecalculateSize();
}

const size_t Screen::Bitmap(ByteArray& arr) const {
    if (arr == nullptr) { arr = new Byte[_bitmapSize]; }

    std::memcpy(arr, _currentCapture, _bitmapSize);

    return _bitmapSize;
}

const size_t Screen::WholeDeal(ByteArray& arr) const {

    if (arr == nullptr) { arr = new Byte[TotalSize()]; }

    std::memcpy(arr, (LPSTR)&_bmpHeader, BMP_FILE_HEADER_SIZE);
    std::memcpy(&arr[BMP_FILE_HEADER_SIZE], (LPSTR)&_bmpInfo, BMP_INFO_HEADER_SIZE);

    std::memcpy(&arr[BMP_HEADER_SIZE], _currentCapture, _bitmapSize);

    return TotalSize();
}

//const DiffArray& Screen::GetDifferences() const {
//    return _differenceArray;
//}

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

    if (arr == nullptr) { arr = new Byte[BMP_HEADER_SIZE]; }

    #if defined(_WIN32)
    std::memcpy(arr, (LPSTR)&_bmpHeader, BMP_FILE_HEADER_SIZE);
    std::memcpy(&arr[BMP_FILE_HEADER_SIZE], (LPSTR)&_bmpInfo, BMP_INFO_HEADER_SIZE);

    #elif defined(__APPLE__)

    std::memcpy(arr, _bitmapHeader, BMP_FILE_HEADER_SIZE);
    std::memcpy(&arr[BMP_FILE_HEADER_SIZE], _bitmapInfo, BMP_INFO_HEADER_SIZE);

    #endif

    return BMP_HEADER_SIZE;
}

