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
using BmpFileHeader = std::array<Byte, BMP_HEADER_SIZE>;

class ScreenCapture {
	
public:
	
    static const BmpFileHeader ConstructBMPHeader(const Resolution& targetRes = RES_480,
        const Ushort bitsPerPixel = 32);  // Initializes values for bitmap header

private:
	
    Resolution _srcResolution;      // Resolution of source screen
    Resolution _targetResolution;   // Target Resolution

    BmpFileHeader _header {};
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

public:

    ScreenCapture();
    ScreenCapture(const size_t, const size_t, const size_t, const size_t);

    ScreenCapture(const ScreenCapture&) = delete;
    ScreenCapture(ScreenCapture&&) = delete;
    ~ScreenCapture();

    void CaptureScreen();  // Capture the screen and store in _currentCapture

    void ReInitialize(const Resolution& targetRes = RES_480);  // Resize the destination screen

    constexpr const size_t TotalSize() const;  // Size of header and data

    const size_t WholeDeal(ByteArray& arr) const;
};

  
