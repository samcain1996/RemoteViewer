//#include <iostream>
//#include <Windows.h>
//#include <fstream>
//
//int main() {
//    BITMAPFILEHEADER _bmpHeader;
//    BITMAPINFOHEADER _bmpInfo;
//
//    _bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
//    _bmpInfo.biWidth = 1440;
//    _bmpInfo.biHeight = 900;
//    _bmpInfo.biPlanes = 1;
//    _bmpInfo.biBitCount = 32;
//    _bmpInfo.biCompression = BI_RGB;
//    _bmpInfo.biSizeImage = 0;
//    _bmpInfo.biXPelsPerMeter = 0;
//    _bmpInfo.biYPelsPerMeter = 0;
//    _bmpInfo.biClrUsed = 0;
//    _bmpInfo.biClrImportant = 0;
//
//    _bmpHeader.bfType = 0x4D42; // BM.
//    _bmpHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
//
//    _bmpHeader.bfSize = 900 * 1440 * 4 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
//
//    char* arr = new char[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 900 * 1440 * 4];
//    std::memcpy(arr, (LPSTR)&_bmpHeader, sizeof(BITMAPFILEHEADER));
//    std::memcpy(&arr[sizeof(BITMAPFILEHEADER)], (LPSTR)&_bmpInfo, sizeof(BITMAPINFOHEADER));
//
//    char* temp = new char[900 * 1440 * 4];
//    std::ifstream in("capture.bmp", std::ios_base::binary);
//    in.read(temp, 900 * 1440 * 4);
//    std::reverse(temp, temp + 900 * 1440 * 4);
//
//    std::memcpy(&arr[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)], temp, 900 * 1440 * 4);
//
//    std::ofstream out("out.bmp", std::ios_base::binary);
//    out.write(arr, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 900 * 1440 * 4);
//    return 0;
//}