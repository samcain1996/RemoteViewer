#include "Capture.h"

ScreenCapture::ScreenCapture(const Resolution& res) : ScreenCapture(res.width, res.height) {}

ScreenCapture::ScreenCapture(const Ushort width, const Ushort height) {

    _resolution.width  = width;
    _resolution.height = height;

#if defined(__linux__)

    _display = XOpenDisplay(nullptr);
    _root = DefaultRootWindow(_display);

    XGetWindowAttributes(_display, _root, &_attributes);

    _resolution.width  = _attributes.width;
    _resolution.height = _attributes.height;
    
#endif

#if defined(_WIN32)

    _srcHDC = GetDC(GetDesktopWindow());  // Get the device context of the monitor [1]
    _memHDC = CreateCompatibleDC(_srcHDC);    // Creates a new device context from previous context

    // Create bitmap from the source using the destination's resolution
    _hScreen = CreateCompatibleBitmap(_srcHDC, _targetResolution.first, _targetResolution.second);

    SelectObject(_memHDC, _hScreen);  // Select bitmap into DC [2]

    // Not likely that source and destination are same resolution.
    // Tell system how to stretch the image
    SetStretchBltMode(_memHDC, HALFTONE);

    _hDIB = NULL;

#endif

    _previousCapture = new Byte[_bitmapSize];

#if defined(__APPLE__)

    _currentCapture = new Byte[CalulcateBMPFileSize(srcWidth, srcHeight)];
    _colorspace = CGColorSpaceCreateDeviceRGB();
    _context = CGBitmapContextCreate(_currentCapture, srcWidth, srcHeight, 
        8, srcWidth * 4, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif

    ReInitialize(_resolution);

}

ScreenCapture::~ScreenCapture() {

#if defined(_WIN32)

    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    DeleteObject(_hScreen);

    ReleaseDC(NULL, _srcHDC);
    DeleteObject(_memHDC);

#elif defined(__APPLE__)

    delete[](ByteArray) _currentCapture;    

    CGImageRelease(_image);
    CGContextRelease(_context);
    CGColorSpaceRelease(_colorspace);

#elif defined(__linux__)

    XDestroyImage(_img);
    XCloseDisplay(_display);

#endif

    if (_previousCapture) {
        delete[](ByteArray)_previousCapture;
    }


}

constexpr const BmpFileHeader ScreenCapture::BaseHeader() {

    BmpFileHeader baseHeader {};  // TODO: See how to init all vals to 0

    baseHeader[0] = 0x42;
    baseHeader[1] = 0x4D;

    baseHeader[10] = 0x36;

    baseHeader[BMP_FILE_HEADER_SIZE] = 0x28;

    baseHeader[BMP_FILE_HEADER_SIZE+12] = 1;

    return baseHeader;
}

const BmpFileHeader ScreenCapture::ConstructBMPHeader(Resolution resolution,
        const Ushort bitsPerPixel) {

    BmpFileHeader header = BaseHeader();

    encode256(&header[2], resolution.width * resolution.height * 
        BMP_CHANNELS + BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE, Endianess::Big);

    encode256(&header[4+BMP_FILE_HEADER_SIZE], resolution.width, Endianess::Big);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    resolution.height = -resolution.height;

#endif

    encode256(&header[8+BMP_FILE_HEADER_SIZE], resolution.height, Endianess::Big);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    std::for_each( (header.begin() + BMP_FILE_HEADER_SIZE + 8), (header.begin() + BMP_FILE_HEADER_SIZE + 12), 
        [](Byte& b) { if (b == NULL) { b = 255; } });

#endif

    header[BMP_FILE_HEADER_SIZE+14] = bitsPerPixel;
	
    return header;
	
}

constexpr const size_t ScreenCapture::TotalSize() const {
    return _bitmapSize + BMP_HEADER_SIZE;
}

void ScreenCapture::ReInitialize(const Resolution& resolution) {

    _resolution = resolution;

    _bitmapSize = CalulcateBMPFileSize(_resolution, _bitsPerPixel);

    #if defined(__APPLE__)

    delete[](ByteArray)_currentCapture;
    _currentCapture = new Byte[_bitmapSize];

    #endif

    delete[](ByteArray)_previousCapture;
    _previousCapture = new Byte[_bitmapSize];

    _header = ConstructBMPHeader(_resolution, _bitsPerPixel);

#if defined(_WIN32)

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, _resolution.width, _resolution.height);
    SelectObject(_memHDC, _hScreen);

    // Free _hDIB and re-lock
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _bitmapSize);
    _currentCapture = GlobalLock(_hDIB);

#endif
	
}

const size_t ScreenCapture::WholeDeal(ByteArray& arr) const {

    const size_t captureSize = TotalSize();

    if (arr == nullptr) { arr = new Byte[captureSize]; }

    std::memcpy(arr, &_header, BMP_HEADER_SIZE);

    std::memcpy(&arr[BMP_HEADER_SIZE], _currentCapture, _bitmapSize);

    return captureSize;
}

const ImageData ScreenCapture::WholeDeal() const {

    ImageData fullImage(TotalSize());
    std::copy(_header.begin(), _header.end(), fullImage.begin());
    std::memcpy(&fullImage[BMP_HEADER_SIZE], _currentCapture, _bitmapSize);

    return fullImage;


}

const size_t ScreenCapture::GetImageData(ByteArray& arr) const {

    if (arr == nullptr) { arr = new Byte[_bitmapSize]; }

    std::memcpy(arr, _currentCapture, _bitmapSize);

    return _bitmapSize;

}

const ImageData ScreenCapture::GetImageData() const {
    ImageData data (_bitmapSize);
    std::memcpy(&data[0], _currentCapture, _bitmapSize);
    return data;
}

void ScreenCapture::CaptureScreen() {

    if (_currentCapture)
    std::memcpy(_previousCapture, _currentCapture, _bitmapSize);

#if defined(_WIN32)

    StretchBlt(_memHDC, 0, 0, _resolution.width, _resolution.height,
        _srcHDC, 0, 0, _resolution.width, _resolution.height, SRCCOPY);

    GetObject(_hScreen, sizeof BITMAP, &_screenBMP);

    // Should be legal because BITMAPINFO has no padding, all its data members are aligned.
    GetDIBits(_memHDC, _hScreen, 0,
        (UINT)_screenBMP.bmHeight,
        _currentCapture,
        (BITMAPINFO*)(&_header[BMP_FILE_HEADER_SIZE]), DIB_RGB_COLORS);

#elif defined(__APPLE__)

    _image = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(_context, CGRectMake(0, 0, _resolution.width, _resolution.height), _image);

#elif defined(__linux__)

    _img = XGetImage(_display, _root, 0, 0, _resolution.width, _resolution.height, AllPlanes, ZPixmap);
    _currentCapture = _img->data;

#endif

}

const Resolution& ScreenCapture::ImageResolution() const { return _resolution; }

