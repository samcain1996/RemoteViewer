#pragma once

#include <cmath>
#include "Types.h"

#if defined(_WIN32)

#define NOMINMAX

#include <Windows.h>
#include <ShellScalingApi.h>

#elif defined(__APPLE__)

#include <ApplicationServices/ApplicationServices.h>

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

#if defined(__APPLE__)
static constexpr OPERATING_SYSTEM OS = OPERATING_SYSTEM::MAC;
#elif defined(__linux__)
static constexpr OPERATING_SYSTEM OS = OPERATING_SYSTEM::LINUX;
#else

static constexpr OPERATING_SYSTEM OS = OPERATING_SYSTEM::WINDOWS;

#endif

constexpr const Ushort NUM_COLOR_CHANNELS = 4;
using ByteSpan = std::span<MyByte, NUM_COLOR_CHANNELS>;


// Convert base 10 number to base 256
constexpr void EncodeAsByte(ByteSpan encodedNumber, const Uint32 numberToEncode) {

    encodedNumber[3] = (numberToEncode >> 24) & 0xFF;
    encodedNumber[2] = (numberToEncode >> 16) & 0xFF;
    encodedNumber[1] = (numberToEncode >> 8) & 0xFF;
    encodedNumber[0] = (numberToEncode) & 0xFF;

}

// BMP Constants
constexpr const Ushort BMP_FILE_HEADER_SIZE = 14;
constexpr const Ushort BMP_INFO_HEADER_SIZE = 40;
constexpr const Ushort BMP_HEADER_BPP_OFFSET = BMP_FILE_HEADER_SIZE + 14;
constexpr const Ushort BMP_HEADER_SIZE = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;

constexpr const Ushort PIXEL_DATA_OFFSET = 10;
constexpr const Ushort COLOR_PLANES_OFFSET = BMP_FILE_HEADER_SIZE + 12;
constexpr const Ushort NUM_COLOR_PLANES = 1;

constexpr const Ushort FILESIZE_OFFSET = 2;
constexpr const Ushort WIDTH_OFFSET = BMP_FILE_HEADER_SIZE + sizeof(int);
constexpr const Ushort HEIGHT_OFFSET = WIDTH_OFFSET + sizeof(int);

// Pixel Constants
constexpr const Ushort BITS_PER_CHANNEL = 8;

// Types
using BmpFileHeader = std::array<MyByte, BMP_HEADER_SIZE>;
using PixelData = std::vector<MyByte>;

/*------------------RESOLUTIONS--------------------*/

struct Resolution {
    int width = 0;
    int height = 0;

    constexpr Resolution(const int x, const int y) : width(x), height(y) {}
    constexpr Resolution(const double x, const double y) : width(x), height(y) {}

    double AspectRatio() const { return width / (double)height; }

    Resolution operator*(const int factor) const {

        const size_t targetArea = width * height * factor;

        const int newHeight = sqrt(width * height * factor / AspectRatio());
        const int newWidth = AspectRatio() * newHeight;

        return { newWidth, newHeight };
    }

    Resolution& operator*=(const int factor) {
        const Resolution newResolution = *this * factor;

        return *this;
    }

    Resolution operator/(const int divisor) const {

        const size_t targetArea = width * height / divisor;

        const int newHeight = sqrt(width * height / (divisor * AspectRatio()));
        const int newWidth = AspectRatio() * newHeight;

        return { newWidth, newHeight };
    }

    Resolution& operator/=(const int divisor) {

        Resolution newResolution = *this / divisor;

        return *this;
    }

    // Comparison operators
    bool operator==(const Resolution& other) const {
        return width == other.width && height == other.height;
    }

    // Change to compare area ??
    bool operator<(const Resolution& other) const {
        return width < other.width || height < other.height;
    }
    bool operator>(const Resolution& other) const {
        return !(*this == other || *this < other);
    }

};

// Test resolutions
constexpr static const Resolution RES_2X2 = { 2, 2 };
constexpr static const Resolution RES_4X4 = { 4, 4 };

// Low definition
constexpr static const Resolution RES_144 = { 256, 144 };
constexpr static const Resolution RES_DEBUG = { 256, 144 };

// Standard definition
constexpr static const Resolution RES_480 = { 640, 480 };

// High definition
constexpr static const Resolution RES_720 = { 1280, 720 };
constexpr static const Resolution RES_1080 = { 1920, 1080 };
constexpr static const Resolution RES_1440 = { 2560, 1440 };
constexpr static const Resolution RES_2K = { 2560, 1440 };
constexpr static const Resolution RES_4K = { 3840, 2160 };

/*--------------------------------------------------*/

struct ScreenArea {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    constexpr ScreenArea() = default;
    constexpr ScreenArea(const int left, const int right, const int top, const int bottom) :
        left(left), right(right), top(top), bottom(bottom) {}
    constexpr ScreenArea(const Resolution& res) : right(res.width), bottom(res.height) {}
    constexpr ScreenArea(const Resolution& res, const int xOffset, const int yOffset) :
        left(xOffset), right(xOffset + res.width), top(yOffset), bottom(yOffset + res.height) {}

    // Total area of screen being captured
    int Area() const { return (right - left) * (bottom - top); }

    // One ScreenArea is smaller than another if its Area is less
    bool operator<(const ScreenArea& other) const {
        return Area() < other.Area();
    }

    explicit operator Resolution() { return { (right - left), (bottom - top) }; }
};

