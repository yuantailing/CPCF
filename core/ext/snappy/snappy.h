#pragma once

#include "../../shared_api/rt/runtime_base.h"

namespace rt
{

extern SSIZE_T	SnappyCompress(LPCVOID in, SIZE_T in_size, LPVOID out, SIZE_T out_size);	// return compressed size, -1 for error
extern bool		SnappyUncompress(LPCVOID in, SIZE_T in_size, LPVOID out);					// out_size must be SnappyGetUncompressedLength
extern SIZE_T	SnappyMaxCompressedLength(SIZE_T in_size);
extern SSIZE_T	SnappyGetUncompressedLength(LPCVOID in, SIZE_T in_size);					// return uncompressed size, -1 for error

};