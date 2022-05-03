#pragma once
#include "Types.h"

#if defined(_WIN32)
#include <Windows.h>

#define WIDTH 1280
#define HEIGHT 720

ByteVec CaptureScreen();
#endif
