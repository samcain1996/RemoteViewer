//#include <lz4.h>
//#include "Types.h"
//
//class Compressor {
//public:
//	static int BLOCK_SIZE;
//	static int compress(ByteArray dataToCompress, size_t sizeInBytes, ByteArray compressed) {
//		LZ4_stream_t* const lz4Stream = LZ4_createStream();
//		const size_t compBufSize = LZ4_COMPRESSBOUND(sizeInBytes);
//
//		char* const compressedBuffer = new char[compBufSize];
//
//		size_t inputPos = 0;
//
//		while (true) {
//			char* const inpPtr = &((char*)dataToCompress)[inputPos];
//
//			if (inputPos >= sizeInBytes) { break; }
//
//			const int compressedBytes = LZ4_compress_fast_continue(
//				lz4Stream, inpPtr, compressedBuffer, BLOCK_SIZE, compBufSize, 1);
//			if (compressedBytes <= 0) { break; }
//
//			std::memcpy(&compressed[inputPos], compressedBuffer, compressedBytes);
//
//			if (compressedBytes != BLOCK_SIZE) return 1;
//
//			inputPos += BLOCK_SIZE;
//		}
//
//		delete[] compressedBuffer;
//		LZ4_freeStream(lz4Stream);
//
//		return 1;
//	}
//	static int decompress(ByteArray);
//};
//
//inline int Compressor::BLOCK_SIZE = 4096;