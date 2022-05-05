#pragma once
#include "Types.h"

#if defined(_WIN32)
#include <Windows.h>
#include <ShellScalingApi.h>
#elif defined(_APPLE_)
#include <ApplicationServices/ApplicationServices.h>
#endif

class Screen {
private:
    ushort _srcWidth, _srcHeight;  // Resolution of source screen
    ushort _dstWidth, _dstHeight;  // Resolution of destination screen

    ByteArray _currentCapture;     // Buffer holding screen capture
    ByteArray _previousCapture;    // Buffer holding previous screen capture

    // Buffer holding the difference between current and previous captures
    ByteArray _differenceMap;      
    size_t _differences;  // Number of differences in differenceMap

#if defined(_WIN32)

    HDC _srcHDC;                   // Device context of source
    HDC _memHDC;                   // Device context of destination

    // Bitmap data
    HBITMAP _hScreen;
    BITMAP _screenBMP;
    DWORD _bitmapSize;
    BITMAPFILEHEADER _bmpHeader;
    BITMAPINFOHEADER _bmpInfo;
    HANDLE _hDIB;
    char* _lpbitmap;

#elif defined(_APPLE_)

    CGColorSpace* _colorspace = nullptr;
    CGContext*    _context    = nullptr;
    CGImage*      _image      = nullptr;

#endif

    void CalculateDifference();  // Calculates the differenceMap
    void InitializeBMPHeader();  // Initializes values for bitmap header
    void RecalculateSize();      // Re-initialize variables after screen resize

public:

    Screen();
    Screen(const ushort, const ushort, const ushort, const ushort);
    ~Screen();

    void CaptureScreen();  // Capture the screen and store in _currentCapture

    void GetHeader(ByteArray);
    void Resize(const ushort width, const ushort height);  // Resize the destination screen

    const size_t TotalSize() const; 
    const ByteArray Bitmap() const;  
    const size_t GetDifferences(ByteArray);
};

  
