#pragma once
#include "Types.h"

#if defined(_WIN32)

#include <Windows.h>
#include <ShellScalingApi.h>

#elif defined(__APPLE__)

#include <ApplicationServices/ApplicationServices.h>

#endif

constexpr const Ushort BMP_FILE_HEADER_SIZE = 14;
constexpr const Ushort BMP_INFO_HEADER_SIZE = 40;
constexpr const Ushort BMP_HEADER_SIZE      = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;

class Screen {
private:
    size_t _srcWidth, _srcHeight;  // Resolution of source screen
    size_t _dstWidth, _dstHeight;  // Resolution of destination screen

    ByteArray _currentCapture;     // Buffer holding screen capture
    ByteArray _previousCapture;    // Buffer holding previous screen capture

    // Buffer holding the difference between current and previous captures
    // DiffArray _differenceArray;      

    DWORD _bitmapSize;

#if defined(_WIN32)

    HDC _srcHDC;                   // Device context of source
    HDC _memHDC;                   // Device context of destination

    // Bitmap data
    HBITMAP _hScreen;
    BITMAP _screenBMP;
    BITMAPFILEHEADER _bmpHeader;
    BITMAPINFOHEADER _bmpInfo;
    HANDLE _hDIB;
    char* _lpbitmap;

#elif defined(__APPLE__)

    Byte _bitmapHeader[BMP_FILE_HEADER_SIZE];
    Byte _bitmapInfo[BMP_INFO_HEADER_SIZE];

    CGColorSpace* _colorspace = nullptr;
    CGContext*    _context    = nullptr;
    CGImage*      _image      = nullptr;

#endif

    void CalculateDifference();  // Calculates the differenceMap
    void InitializeBMPHeader();  // Initializes values for bitmap header
    void RecalculateSize();      // Re-initialize variables after screen resize

public:

    Screen();
    Screen(const size_t, const size_t, const size_t, const size_t);

    Screen(const Screen&) = delete;
    Screen(Screen&&) = delete;
    ~Screen();

    void CaptureScreen();  // Capture the screen and store in _currentCapture

    void Resize(const Ushort width, const Ushort height);  // Resize the destination screen

    const size_t TotalSize() const;  // Size of header and data
    const size_t GetHeader(ByteArray& arr) const;
    const size_t Bitmap(ByteArray& arr) const;  // Return the bitmap data

    const size_t WholeDeal(ByteArray& arr) const;

    // const DiffArray& GetDifferences() const;
};

  
