#pragma once

#include <fstream>
#include "Types.h"

#if defined(_WIN32)

#include <Windows.h>
#include <ShellScalingApi.h>

#elif defined(__APPLE__)

#include <unistd.h>
#include <ApplicationServices/ApplicationServices.h>

#elif defined(__linux__)

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

using Ushort = std::uint16_t;
using Uint32 = std::uint32_t;

// BMP Constants
constexpr const Ushort BMP_FILE_HEADER_SIZE = 14;
constexpr const Ushort BMP_INFO_HEADER_SIZE = 40;
constexpr const Ushort BMP_HEADER_SIZE      = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;
constexpr const Ushort BMP_COLOR_CHANNELS   = 4;

// Types
using BmpFileHeader = std::array<Byte, BMP_HEADER_SIZE>;

using PixelData = Byte*;
using ImageData = std::vector<Byte>;

/*------------------RESOLUTIONS--------------------*/

struct Resolution {
    Ushort width;
    Ushort height;
};

constexpr const Resolution RES_480 = { 640, 480 };
constexpr const Resolution RES_720 = { 1280, 720 };
constexpr const Resolution RES_1080 = { 1920, 1080 };
constexpr const Resolution RES_1440 = { 2560, 1440 };
constexpr const Resolution RES_4K = { 3840, 2160 };

/*--------------------------------------------------*/

class ScreenCapture {
	
public:
	
    static const BmpFileHeader ConstructBMPHeader(Resolution resolution = DefaultResolution,
        const Ushort bitsPerPixel = 32);  // Initializes values for bitmap header

    static const Uint32 CalculateBMPFileSize(const Resolution& resolution = DefaultResolution, const Ushort bitsPerPixel = 32);

private:
	
    Resolution _resolution = DefaultResolution;  
    BmpFileHeader _header {};

    // Buffer holding screen capture 
    ImageData _pixelData {}; 

    Uint32 _bitmapSize   = 0;
    Uint32 _bitsPerPixel = 32;

#if defined(_WIN32)

    RECT rcClient;

    HDC _srcHDC; // Device context of source
    HDC _memHDC; // Device context of destination

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
    Window _root {};
    XWindowAttributes _attributes = { 0 };
    XImage* _image = nullptr;

#endif

    constexpr static const BmpFileHeader BaseHeader();

public:

    static inline Resolution DefaultResolution = RES_720;

private:

    void ReInitialize(const Resolution& res = DefaultResolution);

public:

    ScreenCapture(const ScreenCapture&);
    ScreenCapture(ScreenCapture&&) = delete;

    ScreenCapture(const Resolution& res = DefaultResolution);
    ScreenCapture(const Ushort width, const Ushort height);

    ~ScreenCapture();

    ScreenCapture& operator=(const ScreenCapture&) = delete;
    ScreenCapture& operator=(ScreenCapture&&) = delete;

    void ReSize(const Resolution& res = DefaultResolution);

    const ImageData CaptureScreen(); 
    
    const ImageData WholeDeal() const;
    constexpr const size_t TotalSize() const;

    const Resolution& ImageResolution() const;

    void SaveToFile(std::string filename = "screenshot.bmp") const;
};

  
