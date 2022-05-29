#pragma once
#include "Types.h"

#if defined(_WIN32)

#include <Windows.h>
#include <ShellScalingApi.h>

#elif defined(__APPLE__)

#include <ApplicationServices/ApplicationServices.h>

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

constexpr const Ushort BMP_FILE_HEADER_SIZE = 14;
constexpr const Ushort BMP_INFO_HEADER_SIZE = 40;
constexpr const Ushort BMP_HEADER_SIZE      = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;

using PixelData = void*;

class ScreenCapture {
private:
    size_t _srcWidth, _srcHeight;  // Resolution of source screen
    size_t _dstWidth, _dstHeight;  // Resolution of destination screen

    Byte _header[BMP_HEADER_SIZE];
    PixelData _currentCapture = nullptr;     // Buffer holding screen capture
    PixelData _previousCapture = nullptr;    // Buffer holding previous screen capture  

    DWORD _bitmapSize = 0;
    DWORD _bitsPerPixel = 32;

#if defined(_WIN32)

    HDC _srcHDC;                   // Device context of source
    HDC _memHDC;                   // Device context of destination

    // Bitmap data
    HBITMAP _hScreen;
    BITMAP _screenBMP;
    HANDLE _hDIB;

#endif

#if defined(__APPLE__)

    CGColorSpace* _colorspace = nullptr;
    CGContext*    _context    = nullptr;
    CGImage*      _image      = nullptr;

#elif defined (__linux__) 

    Display* _display = nullptr;
    Window _root;
    XWindowAttributes _attributes = { 0 };
    XImage* _img = nullptr;

#endif

    void InitializeBMPHeader();  // Initializes values for bitmap header
    void RecalculateSize();      // Re-initialize variables after screen resize

public:

    ScreenCapture();
    ScreenCapture(const size_t, const size_t, const size_t, const size_t);

    ScreenCapture(const ScreenCapture&) = delete;
    ScreenCapture(ScreenCapture&&) = delete;
    ~ScreenCapture();

    void CaptureScreen();  // Capture the screen and store in _currentCapture

    void Resize(const Ushort width, const Ushort height);  // Resize the destination screen

    const size_t TotalSize() const;  // Size of header and data

    const size_t WholeDeal(ByteArray& arr) const;
};

  
