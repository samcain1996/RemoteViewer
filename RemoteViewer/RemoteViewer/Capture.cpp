#include "Capture.h"

ByteVec CaptureScreen() {
    ByteVec screenBuf;

	HDC srcHDC = GetDC(GetDesktopWindow());
	HDC memHDC = CreateCompatibleDC(srcHDC);

	HBITMAP hScreen = CreateCompatibleBitmap(srcHDC, WIDTH, HEIGHT);

	SelectObject(memHDC, hScreen);

    SetStretchBltMode(memHDC, HALFTONE);

    StretchBlt(memHDC, 0, 0, WIDTH, HEIGHT, srcHDC, 0, 0, 1920, 1080, SRCCOPY);

	BITMAP screenBMP;
	GetObject(hScreen, sizeof BITMAP, &screenBMP);

    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;
    DWORD dwBytesWritten = 0;
    DWORD dwSizeofDIB = 0;
    HANDLE hFile = NULL;
    char* lpbitmap = NULL;
    HANDLE hDIB = NULL;
    DWORD dwBmpSize = 0;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenBMP.bmWidth;
    bi.biHeight = screenBMP.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((screenBMP.bmWidth * bi.biBitCount + 31) / 32) * 4 * screenBMP.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char*)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer 
    // that's pointed to by lpbitmap.
    GetDIBits(memHDC, hScreen, 0,
        (UINT)screenBMP.bmHeight,
        lpbitmap,
        (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // A file is created, this is where we will save the screen capture.
    /*hFile = CreateFile(L"captureqwsx.bmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);*/

    // Add the size of the headers to the size of the bitmap to get the total file size.
    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.
    //buf = new char[dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
    //memcpy(buf, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER));
    //memcpy(&buf[sizeof(BITMAPFILEHEADER)], (LPSTR)&bi, sizeof(BITMAPINFOHEADER));
    //memcpy(&buf[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)], (LPSTR)lpbitmap, dwBmpSize);
    //WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    std::copy((LPSTR)&bmfHeader, (LPSTR)&bmfHeader + sizeof(BITMAPFILEHEADER), std::back_inserter(screenBuf));
    //WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    std::copy((LPSTR)&bi, (LPSTR)&bi + sizeof(BITMAPINFOHEADER), std::back_inserter(screenBuf));
    //WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
    std::copy((LPSTR)lpbitmap, (LPSTR)lpbitmap + dwBmpSize, std::back_inserter(screenBuf));

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    //CloseHandle(hFile);

    return screenBuf;
}