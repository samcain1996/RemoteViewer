#pragma once

#include <fstream>
#include "QuickShot/ImageTypes.h"

/*--------------------------------------------------*/
class ScreenCapture {

private:

    Resolution _resolution = DefaultResolution;
    ScreenArea _captureArea{};

    // Header needed to create a valid bitmap file
    BmpFileHeader _header{};

    // Bufferhoklbh holding screen capture 
    PixelData _pixelData{};

    Uint32 _captureSize = 0;
    Uint32 _bitsPerPixel = 32;

#if defined(_WIN32)

    HDC _srcHDC; // Device context of source
    HDC _memHDC; // Device context of destination

    // Windows bitmap data
    HBITMAP _hScreen;
    BITMAP _screenBMP;
    HANDLE _hDIB;

#endif

#if defined(__APPLE__)

    CGColorSpace* _colorspace = nullptr;
    CGContext* _context = nullptr;
    CGImage* _image = nullptr;

#elif defined (__linux__) 

    static inline Display* _display = XOpenDisplay(nullptr);
    static inline Window _root = DefaultRootWindow(_display);
    static inline XWindowAttributes _attributes = { 0 };

    XImage* _image = nullptr;

#endif

public:

    static Resolution DefaultResolution;
    static Resolution NativeResolution(const bool reinit = false);
    static inline void CalibrateResolution() { NativeResolution(true); }

public:

    /* ---------- Constructors and Destructor ---------- */

    ScreenCapture(const ScreenCapture&);
    ScreenCapture(ScreenCapture&&) = delete;

    ScreenCapture(const Resolution& res = DefaultResolution, const ScreenArea& areaToCapture = NativeResolution());

    ScreenCapture(const int width, const int height);

    ScreenCapture& operator=(const ScreenCapture&) = delete;
    ScreenCapture& operator=(ScreenCapture&&) = delete;

    ~ScreenCapture();

    /* ------------------------------------------------- */

    void Resize(const Resolution& res = DefaultResolution);
    void Crop(const ScreenArea& area);
    const PixelData& CaptureScreen();

    const PixelData WholeDeal() const;
    const Resolution& GetResolution() const;

    static void SaveToFile(const PixelData& imageAndHeader, std::string filename = "screenshot.bmp");
    static void SaveToFile(const PixelData& image, const BmpFileHeader& header, std::string filename = "screenshot.bmp");
    static void SaveToFile(const PixelData& image, const Resolution& resolution, std::string filename = "screenshot.bmp");
    void SaveToFile(const std::string& filename = "screenshot.bmp") const;
};



constexpr BmpFileHeader BaseHeader() {
    BmpFileHeader baseHeader{};  // All values init to '\0'?

    // Identifies file as bmp
    baseHeader[0] = 0x42;
    baseHeader[1] = 0x4D;

    // Offset of pixel data
    baseHeader[PIXEL_DATA_OFFSET] = BMP_HEADER_SIZE;

    // Size of entire header
    baseHeader[BMP_FILE_HEADER_SIZE] = BMP_INFO_HEADER_SIZE;

    // Number of color planes (must be 1)
    baseHeader[COLOR_PLANES_OFFSET] = NUM_COLOR_PLANES;

    return baseHeader;
}

// Size of a bitmap stored on disk
constexpr Uint32 CalculateBMPFileSize(const Resolution& resolution = ScreenCapture::DefaultResolution, const Ushort bitsPerPixel = 32,
        const bool includeHeader = true) {
    return (includeHeader ? BMP_HEADER_SIZE : 0) + 
        ((resolution.width * bitsPerPixel + 31) / 32) * NUM_COLOR_CHANNELS * resolution.height;
};

static const inline BmpFileHeader ConstructBMPHeader(const Resolution& resolution = ScreenCapture::DefaultResolution,
    const Ushort bitsPerPixel = 32) {

    using HeaderIter = BmpFileHeader::iterator;

    const int filesize = BMP_HEADER_SIZE + CalculateBMPFileSize(resolution, bitsPerPixel);

    const int OS_MODIFIER = OS == OperatingSystem::MAC ? 1 : -1;

    BmpFileHeader header = BaseHeader();

    HeaderIter filesizeIter = header.begin() + FILESIZE_OFFSET;
    HeaderIter widthIter = header.begin() + WIDTH_OFFSET;
    HeaderIter heightIter = header.begin() + HEIGHT_OFFSET;

    // Encode file size
    EncodeAsByte(ByteSpan(filesizeIter, sizeof(filesize)), filesize);

    // Encode pixels wide
    EncodeAsByte(ByteSpan(widthIter, sizeof(resolution.width)), resolution.width);

    // Encode pixels high
    EncodeAsByte(ByteSpan(heightIter, sizeof(resolution.height)), OS_MODIFIER * resolution.height);

    if (OS == OperatingSystem::MAC) {  // Window bitmaps are stored upside down

        std::for_each(heightIter, heightIter + sizeof(resolution.height),
            [](MyByte& b) { if (b == '\0') { b = (MyByte)MAX_MYBYTE_VAL; } });

    }

    header[BMP_HEADER_BPP_OFFSET] = bitsPerPixel;

    return header;
}

class Scaler {
    struct ScaleRatio {
        double xRatio = 1;
        double yRatio = 1;

        ScaleRatio(const double x, const double y) : xRatio(x), yRatio(y) {}
        ScaleRatio(const std::pair<double, double>& ratio) : xRatio(ratio.first), yRatio(ratio.second) {}
        ScaleRatio(const Resolution& ratio) : xRatio(ratio.width), yRatio(ratio.height) {}
    };
    using Coordinate = std::pair<int, int>;

    using Pixel = std::span<MyByte>;
    using ConstPixel = std::span<const MyByte>;
    // Get the ratio in the x and y directions between dest and source images
    ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest) {
        return { (dest.width / (double)source.width), (dest.height / (double)source.height) };
    }
    size_t CoordinateToIndex(const Resolution& res, const Coordinate& coord) {
        return coord.second * res.width + coord.first;
    }

    size_t ConvertIndex(const size_t index, const bool toAbsoluteIndex) {
        return toAbsoluteIndex ? index * 4 : index / 4;
    }

    Coordinate IndexToCoordinate(const Resolution& res, const size_t index) {
        return { index % res.width , index / res.width };
    }

    Pixel GetPixel(PixelData& data, const size_t index, const bool isAbsoluteIndex) {
        const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index, true);
        return Pixel{ data }.subspan(idx, 4);
    }

    ConstPixel GetPixel(const PixelData& data, const size_t index, const bool isAbsoluteIndex) {
        const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index, true);
        return ConstPixel{ data }.subspan(idx, 4);
    }
    void AssignPixel(Pixel& assignee, const ConstPixel& other) {
        for (size_t channel = 0; channel < 4; ++channel) {
            assignee[channel] = other[channel];
        }
    }

    // Upscale using nearest neighbor technique
    PixelData NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest) {

        PixelData scaled(CalculateBMPFileSize(dest));
        const auto [scaleX, scaleY] = GetScaleRatio(src, dest);

        for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += 4) {

            // Convert pixel index to x,y coordinates
            const auto [destX, destY] = IndexToCoordinate(dest, ConvertIndex(absIndex, false));
            Pixel scaledPixel = GetPixel(scaled, absIndex, true);

            // Scale the coordinates
            const Coordinate mappedCoord = { destX / scaleX, destY / scaleY };

            // Convert the coordinates to index
            const size_t indexToMap = CoordinateToIndex(src, mappedCoord);
            ConstPixel sourcePixel = GetPixel(source, indexToMap, false);

            // Set scaledPixel equal to corresponding sourcePixel 
            AssignPixel(scaledPixel, sourcePixel);
        }

        return scaled;
    }
};