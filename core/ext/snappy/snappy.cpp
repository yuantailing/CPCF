#include "snappy.h"

#include "./include/snappy.h"

namespace rt
{

SSIZE_T SnappyCompress(LPCVOID in, SIZE_T in_size, LPVOID out, SIZE_T out_size)
{
	if(out_size < SnappyMaxCompressedLength(in_size))return -1;

	size_t outsize = out_size;
	snappy::RawCompress((LPCSTR)in, in_size, (LPSTR)out, &outsize);
	return outsize;
}

bool SnappyUncompress(LPCVOID in, SIZE_T in_size, LPVOID out)
{
	return snappy::RawUncompress((LPCSTR)in, in_size, (LPSTR)out);
}

SIZE_T SnappyMaxCompressedLength(SIZE_T in_size)
{
	return (SIZE_T)snappy::MaxCompressedLength((size_t)in_size);
}

SSIZE_T	SnappyGetUncompressedLength(LPCVOID in, SIZE_T in_size)
{
	size_t size=0;
	if(snappy::GetUncompressedLength((LPCSTR)in, in_size, &size))
		return size;
	return -1;
}


} // namespace rt


#include "./src/snappy.cc"
#include "./src/snappy-sinksource.cc"

/*
#include "./src/snappy.cc"
#include "./src/snappy.cc"
#include "./src/snappy.cc"
#include "./src/snappy.cc"
#include "./src/snappy.cc"
#include "./src/snappy.cc"
*/