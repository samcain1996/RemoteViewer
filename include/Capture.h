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
constexpr const Ushort BMP_COLOR_CHANNELS   = 4;

using PixelData = void*;
using BmpFileHeader = std::array<Byte, BMP_HEADER_SIZE>;

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


class ScreenCapture {
	
public:
	
    static const BmpFileHeader ConstructBMPHeader(Resolution resolution = RES_1080,
        const Ushort bitsPerPixel = 32);  // Initializes values for bitmap header

    constexpr static const Uint32 CalulcateBMPFileSize(const Resolution& resolution, const Ushort bitsPerPixel = 32);

private:
	
    Resolution _resolution;      
    BmpFileHeader _header {};
    ImageData _imageData {};

    PixelData _currentCapture = nullptr;     // Buffer holding screen capture

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
    XImage* _image = nullptr;

#endif

    constexpr static const BmpFileHeader BaseHeader();

public:

    static inline Resolution DefaultResolution = RES_1080;

public:

    
    ScreenCapture(const ScreenCapture&) = delete;
    ScreenCapture(ScreenCapture&&) = delete;

    ScreenCapture(const Resolution& res = DefaultResolution);
    ScreenCapture(const Ushort width = 1920, const Ushort height = 1080);

    ~ScreenCapture();

    void CaptureScreen();  // Capture the screen and store in _currentCapture

    void ReInitialize(const Resolution& res = DefaultResolution);  // Resize the destination screen

    constexpr const size_t TotalSize() const;  // Size of header and data

    // const size_t WholeDeal(ByteArray& arr) const;
    const ImageData WholeDeal() const;

    // const size_t GetImageData(ByteArray& arr) const;
    const ImageData& GetImageData() const;

    const Resolution& ImageResolution() const;
};

  
