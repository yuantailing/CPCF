#pragma once

//////////////////////////////////////////////////////////////////////
// Cross-Platform Core Foundation (CPCF)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of CPF.  nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////

#ifndef __cplusplus
#error Not compiled as C++, use .mm instead of .m on Mac/iOS
#endif

#include <limits.h>
#include <float.h>
#include <sstream>
#include <time.h>
#include <cmath>

#include "../os/predefines.h"

#if defined(PLATFORM_WIN)
#include <intrin.h>
#endif

#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#include <x86intrin.h>
#endif

#if defined(PLATFORM_LINUX)
#include <cstring>
#include <x86intrin.h>
#endif


///////////////////////////////////////////////////////
// Hash map/set
#if defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_MAC) || (defined(PLATFORM_WIN) && ( _MSC_VER >= 1900 )) || defined(PLATFORM_LINUX)

#if __cplusplus == 199711L && !defined(PLATFORM_WIN)
#include <tr1/unordered_map>
#include <tr1/unordered_set>

namespace rt
{
	template<class _Key, class _Tp,
	class _HashFn = std::tr1::hash<_Key> >
	class hash_map: public std::tr1::unordered_map<_Key, _Tp, _HashFn>
    {   typedef _Key	t_Key; typedef _Tp	t_Value;
        public: hash_map(){}
    };
	template<class _Key,
	class _HashFcn  = std::tr1::hash<_Key> >
	class hash_set: public std::tr1::unordered_set<_Key, _HashFcn>
    {   typedef _Key t_Key;
        public: hash_set(){}
    };
}

#else

#include <unordered_map>
#include <unordered_set>

namespace rt
{
	template<class _Key, class _Tp,
	class _HashFn = std::hash<_Key> >
	class hash_map: public std::unordered_map<_Key, _Tp, _HashFn>
    {   typedef _Key	t_Key; typedef _Tp	t_Value;
        public: hash_map(){}
    };
	template<class _Key,
	class _HashFcn  = std::hash<_Key> >
	class hash_set: public std::unordered_set<_Key, _HashFcn>
    {   typedef _Key t_Key;
        public: hash_set(){}
    };
}

#endif

#else
#include <hash_map>
#include <hash_set>
namespace rt
{
    template<class _Kty, class _Ty,
	class _Tr = std::hash_compare<_Kty, std::less<_Kty> >,
	class _Alloc = std::allocator<std::pair<const _Kty, _Ty> > >
    class hash_map: public std::hash_map<_Kty, _Ty, _Tr, _Alloc>
    {   typedef _Kty t_Key; typedef _Ty t_Value;
        public: hash_map(){}
    };
    template<class _Kty,
	class _Tr = std::hash_compare<_Kty, std::less<_Kty> >,
	class _Alloc = std::allocator<_Kty> >
    class hash_set: public std::hash_set<_Kty, _Tr, _Alloc>
    {   typedef _Kty t_Key;
        public: hash_set(){}
    };
}
#endif

namespace rt
{
namespace _details
{
template<typename T>
struct _LOG_FULLDATA: public T{};
}

template<typename T>
INLFUNC const _details::_LOG_FULLDATA<T>& LOG_FULLDATA(const T& x){ return (const _details::_LOG_FULLDATA<T>&)x; }

enum _tagLogType
{
	LOGTYPE_VERBOSE = 0,
	LOGTYPE_UPDATING,
	LOGTYPE_INFORMATIONAL,
	LOGTYPE_HIGHLIGHT,
	LOGTYPE_WARNING,
	LOGTYPE_ERROR,
	LOGTYPE_LEVEL_MASK = 0xff,
	LOGTYPE_MAX,
	LOGTYPE_IN_CONSOLE = 0x100,
	LOGTYPE_IN_LOGFILE = 0x200,
	LOGTYPE_IN_CONSOLE_FORCE = 0x400,
	LOGTYPE_IN_CONSOLE_PROMPT = 0x800,
};
} // namespace rt

///////////////////////////////////////////////////////
namespace os
{	extern void LogWrite(LPCSTR log, LPCSTR file, int line_num, LPCSTR func, int type);
	extern void LogDisplayInConsole(bool yes);
	extern bool LogIsDisplayInConsole();
}

#ifndef PLATFORM_DISABLE_LOG
	#define __LOG_TYPE(x,type)	{	 \
									std::ostringstream _ostr;							\
									_ostr<<x;											\
									::os::LogWrite(	_ostr.rdbuf()->str().c_str(),__FILE__,__LINE__,__FUNCTION__, type); \
								}
    #define _LOGNL				{ ::os::LogWrite("",__FILE__,__LINE__,__FUNCTION__, rt::LOGTYPE_VERBOSE); }
#else
	#define __LOG_TYPE(x,type)	{}
	#define _LOGNL				{}
#endif


// Log to both console and file
#define _LOG_VERBOSE(x)				__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_VERBOSE)
#define _LOG_UPDATING(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_UPDATING)
#define _LOG(x)						__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_INFORMATIONAL)
#define _LOG_HIGHLIGHT(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_HIGHLIGHT)
#define _LOG_WARNING(x)				__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_WARNING)
#define _LOG_ERROR(x)				__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_ERROR)

#define _LOG_POS					__LOG_TYPE(__FUNCTION__<<":L"<<__LINE__<<":"<<__FILE__, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_VERBOSE)
#define _LOG_POS_WARNING			__LOG_TYPE(__FUNCTION__<<":L"<<__LINE__<<":"<<__FILE__, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_WARNING)

// Only Log to console
#define _LOGC_VERBOSE(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_VERBOSE)
#define _LOGC(x)					__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_INFORMATIONAL)
#define _LOGC_PROMPT()				__LOG_TYPE("", rt::LOGTYPE_IN_CONSOLE_PROMPT|rt::LOGTYPE_INFORMATIONAL)
#define _LOGC_HIGHLIGHT(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_HIGHLIGHT)
#define _LOGC_WARNING(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_WARNING)
#define _LOGC_ERROR(x)				__LOG_TYPE(x, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_ERROR)
			
#define _LOGC_POS					__LOG_TYPE(__FUNCTION__<<":L"<<__LINE__<<":"<<__FILE__, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_VERBOSE)
#define _LOGC_POS_WARNING			__LOG_TYPE(__FUNCTION__<<":L"<<__LINE__<<":"<<__FILE__, rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_WARNING)

// Only Log to file
#define _LOGF_VERBOSE(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_VERBOSE)
#define _LOGF(x)					__LOG_TYPE(x, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_INFORMATIONAL)
#define _LOGF_HIGHLIGHT(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_HIGHLIGHT)
#define _LOGF_WARNING(x)			__LOG_TYPE(x, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_WARNING)
#define _LOGF_ERROR(x)				__LOG_TYPE(x, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_ERROR)

#define _LOGF_POS					__LOG_TYPE(__FUNCTION__<<":L"<<__LINE__<<":"<<__FILE__, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_VERBOSE)
#define _LOGF_POS_WARNING			__LOG_TYPE(__FUNCTION__<<":L"<<__LINE__<<":"<<__FILE__, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_WARNING)

#define _LOGP						os::LogPrefix()


///////////////////////////////////////////////////////
// Debugging and error handling
#ifndef ASSERT
	#ifdef PLATFORM_DEBUG_BUILD
		#if defined(PLATFORM_WIN)
			#define ASSERT(x) if(!(x)){ _LOG_ERROR("ASSERTION FAILED: " #x); _LOG_POS; _ASSERT(x); }
		#elif defined(PLATFORM_ANDROID)
			#define ASSERT(x) if(!(x)){ _LOG_ERROR("ASSERTION FAILED: " #x); _LOG_POS; }
		#else
			#include <assert.h>
			#define ASSERT(x) assert(x)
		#endif
	#else
		#define ASSERT(x) {}
	#endif
#endif


#ifdef VERIFY
#undef VERIFY
#endif

#define VERIFY(x)	rt::_details::_VERIFY_RET(x, #x, __FUNCTION__ , __LINE__ , __FILE__)

#define ASSERTH(re) ASSERT(SUCCEEDED(re))
#define VERIFYH(re) VERIFY(SUCCEEDED(re))

namespace rt
{
namespace _details
{
template<typename RET>
INLFUNC RET _VERIFY_RET(RET&& exp, LPCSTR exp_str, LPCSTR func, int line, LPCSTR file)
{
#ifdef PLATFORM_DEBUG_BUILD
	if(!exp)
	{
		_LOG_ERROR("ASSERTION FAILED: "<<exp_str);
		__LOG_TYPE(func<<":L"<<line<<":"<<file, rt::LOGTYPE_IN_LOGFILE|rt::LOGTYPE_VERBOSE);
		ASSERT(0);
	}
#endif
	return exp;
}

} // namespace _details
} // namespace rt

///////////////////////////////////////////////////////

#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#define ASSERT_STATIC(x)	static int MARCO_JOIN(__assertion_failed__,__COUNTER__)[((int)((bool)(x)))*2 - 1] __attribute__((unused));
#else
#define ASSERT_STATIC(x)	static int MARCO_JOIN(__assertion_failed__,__COUNTER__)[((int)((bool)(x)))*2 - 1];
#endif

///////////////////////////////////////////////////////
// Runtime diagnosis
#if defined(PLATFORM_DEBUG_BUILD) && defined(PLATFORM_WIN)
	#define _CheckHeap	ASSERT(_CrtCheckMemory())
#else
	#define _CheckHeap	{}
#endif

#ifdef PLATFORM_DEBUG_BUILD
	#define ASSERT_FLOAT(x)	{ ASSERT(rt::IsNumberOk(x)); }
	#define VERIFY_FLOAT(x)	ASSERT_FLOAT(x)
#else
	#define ASSERT_FLOAT(x)	while(0)
	#define VERIFY_FLOAT(x)	while(x){ break; }
#endif


///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// Some constant 
namespace rt
{
 
struct _DummyType;

template<typename T>
INLFUNC T Sqr(T x){ return x*x; }

template<typename T>
INLFUNC T max(const T& x, const T& y)
{
#pragma warning(disable:4018)
	return (x<y? y:x);
#pragma warning(default:4018)
}
    
template<typename T>
INLFUNC T min(const T& x, const T& y)
{
#pragma warning(disable:4018)
	return (x>y? y:x);
#pragma warning(default:4018)
}
	
template<typename T>
INLFUNC T abs(const T& x)
{
#pragma warning(disable:4018)
	return (x>=0? x:-x);
#pragma warning(default:4018)
}

template<typename T, typename T_R>
INLFUNC bool IsInRange_OO(T var,T_R min_v,T_R max_v)
{
	return (var > min_v) && (var < max_v);
}

template<typename T, typename T_R>
INLFUNC bool IsInRange_CC(T var,T_R min_v,T_R max_v)
{
	return (var >= min_v) && (var <= max_v);
}

template<typename T, typename T_R>
INLFUNC bool IsInRange_CO(T var,T_R min_v,T_R max_v)
{
	return (var >= min_v) && (var < max_v);
}

template<typename T, typename T_R>
INLFUNC bool IsInRange_OC(T var,T_R min_v,T_R max_v)
{
	return (var > min_v) && (var <= max_v);
}

#if defined(PLATFORM_WIN)
FORCEINL bool IsNumberOk				(float val){ return ::rt::IsInRange_CC(_fpclass(val),_FPCLASS_NN,_FPCLASS_PN); }
FORCEINL bool IsNumberOk				(double val){ return ::rt::IsInRange_CC(_fpclass(val),_FPCLASS_NN,_FPCLASS_PN); }
#elif defined(PLATFORM_ANDROID)
FORCEINL bool IsNumberOk				(float val){ return __isnormalf(val); }
FORCEINL bool IsNumberOk				(double val){ return __isnormal(val); }
#else
FORCEINL bool IsNumberOk				(float val){ int c = std::fpclassify(val); return c!=FP_INFINITE && c!=FP_NAN; }
FORCEINL bool IsNumberOk				(double val){ int c = std::fpclassify(val); return c!=FP_INFINITE && c!=FP_NAN; }
#endif

FORCEINL bool IsPositiveInfinity		(float val){ return ((DWORD&)val)==0x7f800000; }
FORCEINL bool IsNegativeInfinity		(float val){ return ((DWORD&)val)==0xff800000; }
FORCEINL bool IsNotANumber				(float val){ return ((DWORD&)val)==0x7fffffff; }
FORCEINL bool IsPositiveInfinity		(double val){ return ((ULONGLONG&)val)==0x7ff0000000000000; }
FORCEINL bool IsNegativeInfinity		(double val){ return ((ULONGLONG&)val)==0xfff0000000000000; }
FORCEINL bool IsNotANumber				(double val){ return ((ULONGLONG&)val)==0x7fffffffffffffff; }
FORCEINL void MakePositiveInfinity		(float& val){ ((DWORD&)val)=0x7f800000; }
FORCEINL void MakeNegativeInfinity		(float& val){ ((DWORD&)val)=0xff800000; }
FORCEINL void MakeNotANumber			(float& val){ ((DWORD&)val)=0x7fffffff; }
FORCEINL void MakePositiveInfinity		(double& val){ ((ULONGLONG&)val)=0x7ff0000000000000; }
FORCEINL void MakeNegativeInfinity		(double& val){ ((ULONGLONG&)val)=0xfff0000000000000; }
FORCEINL void MakeNotANumber			(double& val){ ((ULONGLONG&)val)=0x7fffffffffffffff; }

// PopCount
namespace _details
{
const ULONGLONG m1  = 0x5555555555555555ULL; //binary: 0101...
const ULONGLONG m2  = 0x3333333333333333ULL; //binary: 00110011..
const ULONGLONG m4  = 0x0f0f0f0f0f0f0f0fULL; //binary:  4 zeros,  4 ones ...
};

FORCEINL int PopCount(ULONGLONG x) {
    x -= (x >> 1) & _details::m1;             //put count of each 2 bits into those 2 bits
    x = (x & _details::m2) + ((x >> 2) & _details::m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & _details::m4;        //put count of each 8 bits into those 8 bits 
    x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
    x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
    x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
    return x & 0x7f;
}



// xxHash
namespace _details
{
	static const UINT PRIME1 =  2654435761U;
	static const UINT PRIME2 =  2246822519U;
	static const UINT PRIME3 =  3266489917U;
	static const UINT PRIME4 =   668265263U;
	static const UINT PRIME5 =  0x165667b1U;

	// This version is for very small inputs (< 16  bytes)
	FORCEINL unsigned int xxHash_small(const void* key, int len, unsigned int seed)
	{	const unsigned char* p = (unsigned char*)key;
		const unsigned char* const bEnd = p + len;
		unsigned int idx = seed + PRIME1;
		unsigned int crc = PRIME5;
		const unsigned char* const limit = bEnd - 4;
		while (p<limit)
		{	crc += ((*(unsigned int*)p) + idx++);
			crc += _rotl(crc, 17) * PRIME4;
			crc *= PRIME1;
			p+=4;
		}
		while (p<bEnd)
		{	crc += ((*p) + idx++);
			crc *= PRIME1;
			p++;
		}
		crc += len;
		crc ^= crc >> 15;
		crc *= PRIME2;
		crc ^= crc >> 13;
		crc *= PRIME3;
		crc ^= crc >> 16;
		return crc;
	}
}

FORCEINL UINT xxHash(const void* input, int len, unsigned int seed = 0)
{	// Special case, for small inputs
	if (len < 16) return _details::xxHash_small(input, len, seed);
	const unsigned char* p = (const unsigned char*)input;
	const unsigned char* const bEnd = p + len;
	unsigned int v1 = seed + _details::PRIME1;
	unsigned int v2 = v1 * _details::PRIME2 + len;
	unsigned int v3 = v2 * _details::PRIME3;
	unsigned int v4 = v3 * _details::PRIME4;	
	const unsigned char* const limit = bEnd - 16;
	unsigned int crc;
	while (p<limit)
	{	v1 = _rotl(v1, 13) + (*(unsigned int*)p); p+=4;
		v2 = _rotl(v2, 11) + (*(unsigned int*)p); p+=4;
		v3 = _rotl(v3, 17) + (*(unsigned int*)p); p+=4;
		v4 = _rotl(v4, 19) + (*(unsigned int*)p); p+=4;
	} 

	p = bEnd - 16;
	v1 += _rotl(v1, 17); v2 += _rotl(v2, 19); v3 += _rotl(v3, 13); v4 += _rotl(v4, 11); 
	v1 *= _details::PRIME1; v2 *= _details::PRIME1; v3 *= _details::PRIME1; v4 *= _details::PRIME1; 
	v1 += *(unsigned int*)p; p+=4; v2 += *(unsigned int*)p; p+=4; v3 += *(unsigned int*)p; p+=4; v4 += *(unsigned int*)p;   // p+=4;
	v1 *= _details::PRIME2; v2 *= _details::PRIME2; v3 *= _details::PRIME2; v4 *= _details::PRIME2; 
	v1 += _rotl(v1, 11); v2 += _rotl(v2, 17); v3 += _rotl(v3, 19); v4 += _rotl(v4, 13); 
	v1 *= _details::PRIME3; v2 *= _details::PRIME3; v3 *= _details::PRIME3; v4 *= _details::PRIME3;

	crc = v1 + _rotl(v2, 3) + _rotl(v3, 6) + _rotl(v4, 9);
	crc ^= crc >> 11;
	crc += (_details::PRIME4+len) * _details::PRIME1;
	crc ^= crc >> 15;
	crc *= _details::PRIME2;
	crc ^= crc >> 13;

	return crc;
}

FORCEINL UINT xxHashStrong(const void* input, int len, unsigned int seed = 0)
{	// Special case, for small inputs
	if (len < 16) return _details::xxHash_small(input, len, seed);
	const unsigned char* p = (const unsigned char*)input;
	const unsigned char* const bEnd = p + len;
	unsigned int v1 = seed + _details::PRIME1;
	unsigned int v2 = v1 * _details::PRIME2 + len;
	unsigned int v3 = v2 * _details::PRIME3;
	unsigned int v4 = v3 * _details::PRIME4;	
	const unsigned char* const limit = bEnd - 16;
	unsigned int crc;

	while (p<limit)
	{
		v1 += _rotl(v1, 13); v1 *= _details::PRIME1; v1 += (*(unsigned int*)p); p+=4;
		v2 += _rotl(v2, 11); v2 *= _details::PRIME1; v2 += (*(unsigned int*)p); p+=4;
		v3 += _rotl(v3, 17); v3 *= _details::PRIME1; v3 += (*(unsigned int*)p); p+=4;
		v4 += _rotl(v4, 19); v4 *= _details::PRIME1; v4 += (*(unsigned int*)p); p+=4;
	} 

	p = bEnd - 16;
	v1 += _rotl(v1, 17); v2 += _rotl(v2, 19); v3 += _rotl(v3, 13); v4 += _rotl(v4, 11); 
	v1 *= _details::PRIME1; v2 *= _details::PRIME1; v3 *= _details::PRIME1; v4 *= _details::PRIME1; 
	v1 += *(unsigned int*)p; p+=4; v2 += *(unsigned int*)p; p+=4; v3 += *(unsigned int*)p; p+=4; v4 += *(unsigned int*)p;   // p+=4;
	v1 *= _details::PRIME2; v2 *= _details::PRIME2; v3 *= _details::PRIME2; v4 *= _details::PRIME2; 
	v1 += _rotl(v1, 11); v2 += _rotl(v2, 17); v3 += _rotl(v3, 19); v4 += _rotl(v4, 13); 
	v1 *= _details::PRIME3; v2 *= _details::PRIME3; v3 *= _details::PRIME3; v4 *= _details::PRIME3;

	crc = v1 + _rotl(v2, 3) + _rotl(v3, 6) + _rotl(v4, 9);
	crc ^= crc >> 11;
	crc += (_details::PRIME4+len) * _details::PRIME1;
	crc ^= crc >> 15;
	crc *= _details::PRIME2;
	crc ^= crc >> 13;
	return crc;
}


struct _File
{
	static const int Seek_Begin		= 0;
	static const int Seek_Current	= 1;
	static const int Seek_End		= 2;

	virtual SIZE_T		Read(LPVOID lpBuf,SIZE_T nCount) = 0;
	virtual SIZE_T		Write(LPCVOID lpBuf,SIZE_T nCount) = 0;
	virtual SIZE_T		Seek(SSIZE_T offset,UINT nFrom = Seek_Begin) = 0; // return ULLONG_MAX for failure.
	virtual SIZE_T		GetLength() const = 0;
};

namespace _details
{

template<int Size, SIZE_T set> struct _PodFill
{	static FORCEINL void Fill(LPBYTE p)
	{	*(SIZE_T*)p = set;
		_PodFill<Size-sizeof(SIZE_T), set>::Fill(p+sizeof(SIZE_T));
}	};
	template<SIZE_T set> struct _PodFill<0, set>{ static FORCEINL void Fill(LPBYTE p){}};
	template<SIZE_T set> struct _PodFill<1, set>{ static FORCEINL void Fill(LPBYTE p){ *p = (BYTE)set; }};
	template<SIZE_T set> struct _PodFill<2, set>{ static FORCEINL void Fill(LPBYTE p){ *(WORD*)p = (WORD)set; }};
	template<SIZE_T set> struct _PodFill<3, set>{ static FORCEINL void Fill(LPBYTE p){ *(WORD*)p = (WORD)set; p[2] = (BYTE)set; }};
#if defined(PLATFORM_64BIT)
	template<SIZE_T set> struct _PodFill<4, set>{ static FORCEINL void Fill(LPBYTE p){ *(DWORD*)p = (DWORD)set; }};
	template<SIZE_T set> struct _PodFill<5, set>{ static FORCEINL void Fill(LPBYTE p){ *(DWORD*)p = (DWORD)set; _PodFill<1, set>::Fill(p+4); }};
	template<SIZE_T set> struct _PodFill<6, set>{ static FORCEINL void Fill(LPBYTE p){ *(DWORD*)p = (DWORD)set; _PodFill<2, set>::Fill(p+4); }};
	template<SIZE_T set> struct _PodFill<7, set>{ static FORCEINL void Fill(LPBYTE p){ *(DWORD*)p = (DWORD)set; _PodFill<3, set>::Fill(p+4); }};
#endif

} // namespace _details

template<typename T> FORCEINL void Zero(T& obj){ _details::_PodFill<sizeof(T), 0>::Fill((LPBYTE)&obj); }
template<typename T> FORCEINL void Void(T& obj){ _details::_PodFill<sizeof(T), (SIZE_T)-1>::Fill((LPBYTE)&obj); }
template<UINT LEN> FORCEINL void Zero(LPVOID obj){ _details::_PodFill<LEN, 0>::Fill((LPBYTE)obj); }
template<UINT LEN> FORCEINL void Void(LPVOID obj){ _details::_PodFill<LEN, (SIZE_T)-1>::Fill((LPBYTE)obj); }

FORCEINL void Zero(LPVOID obj, SIZE_T size)
{	SIZE_T* p = (SIZE_T*)obj;
	for(;size>=sizeof(SIZE_T);size-=sizeof(SIZE_T), p++)*p = 0;
	switch(size)
	{	case 0: return;
		case 1: _details::_PodFill<1, 0>::Fill((LPBYTE)p); return;
		case 2: _details::_PodFill<2, 0>::Fill((LPBYTE)p); return;
		case 3: _details::_PodFill<3, 0>::Fill((LPBYTE)p); return;
#if defined(PLATFORM_64BIT)
		case 4: _details::_PodFill<4, 0>::Fill((LPBYTE)p); return;
		case 5: _details::_PodFill<5, 0>::Fill((LPBYTE)p); return;
		case 6: _details::_PodFill<6, 0>::Fill((LPBYTE)p); return;
		case 7: _details::_PodFill<7, 0>::Fill((LPBYTE)p); return;
#endif
		default: ASSERT(0); return;
	}
}

FORCEINL void Void(LPVOID obj, SIZE_T size)
{	SIZE_T* p = (SIZE_T*)obj;
	for(;size>=sizeof(SIZE_T);size-=sizeof(SIZE_T), p++)*p = (SIZE_T)-1;
	switch(size)
	{	case 0: return;
		case 1: _details::_PodFill<1, (SIZE_T)-1>::Fill((LPBYTE)p); return;
		case 2: _details::_PodFill<2, (SIZE_T)-1>::Fill((LPBYTE)p); return;
		case 3: _details::_PodFill<3, (SIZE_T)-1>::Fill((LPBYTE)p); return;
#if defined(PLATFORM_64BIT)
		case 4: _details::_PodFill<4, (SIZE_T)-1>::Fill((LPBYTE)p); return;
		case 5: _details::_PodFill<5, (SIZE_T)-1>::Fill((LPBYTE)p); return;
		case 6: _details::_PodFill<6, (SIZE_T)-1>::Fill((LPBYTE)p); return;
		case 7: _details::_PodFill<7, (SIZE_T)-1>::Fill((LPBYTE)p); return;
#endif
		default: ASSERT(0); return;
	}
}

namespace _details
{

template<int Size> struct _PodCopy
{	static FORCEINL void Fill(LPBYTE p, LPCBYTE s)
	{	*(SIZE_T*)p = *(SIZE_T*)s;
		_PodCopy<Size-sizeof(SIZE_T)>::Fill(p+sizeof(SIZE_T), s+sizeof(SIZE_T));
}	};
	template<> struct _PodCopy<0>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){}};
	template<> struct _PodCopy<1>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){ *p = *s; }};
	template<> struct _PodCopy<2>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){ *(WORD*)p = *(WORD*)s; }};
	template<> struct _PodCopy<3>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){ *(WORD*)p = *(WORD*)s; p[2] = s[2]; }};
#if defined(PLATFORM_64BIT)
	template<> struct _PodCopy<4>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){ *(DWORD*)p = *(DWORD*)s; }};
	template<> struct _PodCopy<5>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){ *(DWORD*)p = *(DWORD*)s; _PodCopy<1>::Fill(p+4, s+4); }};
	template<> struct _PodCopy<6>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){ *(DWORD*)p = *(DWORD*)s; _PodCopy<2>::Fill(p+4, s+4); }};
	template<> struct _PodCopy<7>{ static FORCEINL void Fill(LPBYTE p, LPCBYTE s){ *(DWORD*)p = *(DWORD*)s; _PodCopy<3>::Fill(p+4, s+4); }};
#endif

} // namespace _details

template<UINT LEN>
FORCEINL void Copy(LPVOID obj, LPCVOID from){ _details::_PodCopy<LEN>::Fill((LPBYTE)obj, (LPCBYTE)from); }
template<typename T1, typename T2>
FORCEINL void Copy(T1& obj, const T2& from){ ASSERT_STATIC(sizeof(T1) == sizeof(T2)); Copy<sizeof(T1)>((LPBYTE)&obj, (LPCBYTE)&from); }

namespace _details
{
	// FNV Hash function (http://isthe.com/chongo/tech/comp/fnv/)
#if defined(PLATFORM_64BIT)
	static const SIZE_T	_HashInit = 14695981039346656037ULL;
	INLFUNC SIZE_T		_HashKernel(SIZE_T new_data, SIZE_T prev_val){ return 1099511628211ULL * (prev_val ^ new_data); }
#else
	static const SIZE_T	_HashInit = 2166136261U;
	INLFUNC SIZE_T		_HashKernel(SIZE_T new_data, SIZE_T prev_val){ return 16777619U * (prev_val ^ new_data); }
#endif

	template<int Size=0> struct _HashValueFix
	{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit)
		{	return _HashValueFix<Size-sizeof(SIZE_T)>::Val(p+sizeof(SIZE_T), _HashKernel(*(SIZE_T*)p, prev_val));
	}	};
		template<> struct _HashValueFix<0>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return prev_val; }};
		template<> struct _HashValueFix<1>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return _HashKernel((SIZE_T)*p, prev_val); }};
		template<> struct _HashValueFix<2>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return _HashKernel((SIZE_T)*(WORD*)p, prev_val); }};
		template<> struct _HashValueFix<3>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return _HashKernel((0xffffffU&(SIZE_T)*(DWORD*)p), prev_val); }};
#if defined(PLATFORM_64BIT)
		template<> struct _HashValueFix<4>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return _HashKernel((SIZE_T)*(DWORD*)p, prev_val); }};
		template<> struct _HashValueFix<5>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return _HashKernel(0xffffffffffULL&(SIZE_T)*(ULONGLONG*)p, prev_val); }};
		template<> struct _HashValueFix<6>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return _HashKernel(0xffffffffffffULL&(SIZE_T)*(ULONGLONG*)p, prev_val); }};
		template<> struct _HashValueFix<7>{	static INLFUNC SIZE_T Val(LPCBYTE p, SIZE_T prev_val = _HashInit){ return _HashKernel(0xffffffffffffffULL&(SIZE_T)*(ULONGLONG*)p, prev_val); }};
	// x64
	INLFUNC SIZE_T _HashValue(LPCVOID p_in, SIZE_T size, SIZE_T val = _HashInit)
	{	const SIZE_T* p = (const SIZE_T*)p_in;
		for(; size >= sizeof(SIZE_T); size -=sizeof(SIZE_T), p++)
			val = _HashKernel(*p, val);
		switch(size)
		{	case 0: return _HashValueFix<0>::Val((LPCBYTE)p, val);
			case 1: return _HashValueFix<1>::Val((LPCBYTE)p, val);
			case 2: return _HashValueFix<2>::Val((LPCBYTE)p, val);
			case 3: return _HashValueFix<3>::Val((LPCBYTE)p, val);
			case 4: return _HashValueFix<4>::Val((LPCBYTE)p, val);
			case 5: return _HashValueFix<5>::Val((LPCBYTE)p, val);
			case 6: return _HashValueFix<6>::Val((LPCBYTE)p, val);
			case 7: return _HashValueFix<7>::Val((LPCBYTE)p, val);
			default: ASSERT(0); return val;
		}
	}
#else
	// x86
	INLFUNC SIZE_T _HashValue(LPCVOID p_in, SIZE_T size, SIZE_T val = _HashInit)
	{	const SIZE_T* p = (const SIZE_T*)p_in;
		for(; size >= sizeof(SIZE_T); size -=sizeof(SIZE_T), p++)
			val = _HashKernel(*p, val);
		switch(size)
		{	case 0: return _HashValueFix<0>::Val((LPCBYTE)p, val);
			case 1: return _HashValueFix<1>::Val((LPCBYTE)p, val);
			case 2: return _HashValueFix<2>::Val((LPCBYTE)p, val);
			case 3: return _HashValueFix<3>::Val((LPCBYTE)p, val);
			default: ASSERT(0); return val;
		}
	}
#endif
	template<typename T>
	INLFUNC SIZE_T _HashValuePod(const T& obj, SIZE_T val = _HashInit)
	{	return _HashValueFix<sizeof(T)>::Val((LPCBYTE)&obj, val);
	}
	template<typename T>
	struct hash_compare_str
	{	// traits class for hash containers
		enum // parameters for hash table
		{	bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8		// min_buckets = 2 ^^ N, 0 < N
		};
		INLFUNC size_t operator()(const T& key) const
		{	return _HashValue(key.Begin(),(key.End() - key.Begin())*sizeof(*key.Begin()));	
        }
		INLFUNC bool operator()(const T& _Keyval1, const T& _Keyval2) const
		{	return _Keyval1 < _Keyval2;		}
	};
	//template<typename T, int t_Block = sizeof(T)%4>
	template<typename T, int t_Block = 1>
	struct hash_compare_fix;
	template<typename T> struct hash_compare_fix<T, 1>
	{	
		enum // parameters for hash table
		{	bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8		// min_buckets = 2 ^^ N, 0 < N
		};
		INLFUNC size_t operator()(const T& key) const
		{	return _HashValueFix<sizeof(T)>::Val((LPCBYTE)&key);}
		INLFUNC bool operator()(const T& _Keyval1, const T& _Keyval2) const
		{	return memcmp(&_Keyval1, &_Keyval2, sizeof(T)) < 0;	}
	};
}


} // namespace rt


///////////////////////////////////////////////////////
// Safe delete Objects
#define _SafeRelease(x)				{ if(x){x->Release(); x=nullptr;} }
#define _SafeRelease_ConstPtr(x)	{ if(x)rt::_CastToNonconst(x)->Release(); }

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
#define _SafeCFRelease(x)	{ if(x){::CFRelease(x); x=nullptr;} }
#endif
///////////////////////////////////////////////////////

namespace rt
{
namespace _details
{
template<typename T>
LPSTR __alloca_string(LPSTR p, const T& x)
{	p[x.CopyTo(p)] = 0;
	return p;
}
}} // namespace rt::_details

#define ALLOCA_C_STRING(x)	(rt::_details::__alloca_string((LPSTR)alloca((x).GetLength() + 1), (x)))	// x should be a varible, instead of a expression. use auto x = ..... if it need to be an expression


namespace os
{
namespace _details
{
#ifdef PLATFORM_DEBUG_BUILD	
extern LPVOID	TrackMemoryAllocation(LPVOID p, SIZE_T sz, bool no_ctor, LPCSTR type, UINT co, LPCSTR fn, LPCSTR func, UINT line);
extern void		UntrackMemoryAllocation(LPCVOID p);
extern void		DumpTrackedMemoryAllocation(bool verbose = false);
template<typename T>
T*				TrackMemoryNew(T* p, LPCSTR type, LPCSTR fn, LPCSTR func, UINT line){ TrackMemoryAllocation(p, sizeof(T), false, type, 1, fn, func, line); return p; }
#endif

//////////////////////////////////////////////////////////
// 32byte Aligned Memory allocation and free 
//
// 1. use 32byte-Aligned Memory is optimized for cache hitting rate on CPUs of PIII and above
// 2. Add prefix and suffix bytes to allocated memory block to detect buffer overflow
extern LPVOID	Malloc32AL(size_t size, bool allow_fail);   //size in byte
extern void		Free32AL(LPCVOID ptr_in);
}

extern void EnableMemoryExceptionInThread(bool throw_e);  // default is disabled
extern bool IsMemoryExceptionEnabledInThread();

} // namespace os::_details


#ifdef PLATFORM_DEBUG_BUILD

#define _Malloc8AL(type, co)		_Malloc32AL(type, co)
#define _Malloc32AL(type, co)		((type*)os::_details::TrackMemoryAllocation(os::_details::Malloc32AL(sizeof(type)*(co), true), sizeof(type)*(co), true, #type , (UINT)(co), __FILE__, __FUNCTION__, __LINE__))

#define _SafeFree8AL_ConstPtr(ptr)	_SafeFree32AL_ConstPtr(ptr)
#define _SafeFree32AL_ConstPtr(ptr)	{ os::_details::UntrackMemoryAllocation(ptr); os::_details::Free32AL(ptr); }
#define _SafeDel_ConstPtr(x)		{ if(x){ os::_details::UntrackMemoryAllocation(x); delete x; } }
#define _SafeDelArray_ConstPtr(x)	{ if(x){ os::_details::UntrackMemoryAllocation(x); delete [] x; } }

#define _New(...)					(os::_details::TrackMemoryNew(os::IsMemoryExceptionEnabledInThread()? new __VA_ARGS__ : new (std::nothrow) __VA_ARGS__, #__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__))
#define _NewArray(type, co)			((type*)os::_details::TrackMemoryAllocation(os::IsMemoryExceptionEnabledInThread()? new type[co] : new (std::nothrow) type[co], sizeof(type)*co, false, #type, (UINT)(co), __FILE__, __FUNCTION__, __LINE__))

#define _DumpMemoryAllocations		{ os::_details::DumpTrackedMemoryAllocation(true); }

#else

#define _Malloc8AL(type, co)		((type*) os::IsMemoryExceptionEnabledInThread()? new BYTE[sizeof(type)*co] : new (std::nothrow) BYTE[sizeof(type)*co])
#define _Malloc32AL(type, co)		((type*)os::_details::Malloc32AL(sizeof(type)*(co), true))

#define _SafeFree8AL_ConstPtr(ptr)	{ delete [] (LPBYTE)ptr; }
#define _SafeFree32AL_ConstPtr(ptr)	{ os::_details::Free32AL(ptr); }
#define _SafeDel_ConstPtr(x)		{ if(x){delete x; } }
#define _SafeDelArray_ConstPtr(x)	{ if(x){delete [] x; } }

#define _New(...)					(os::IsMemoryExceptionEnabledInThread()? new __VA_ARGS__ : new (std::nothrow) __VA_ARGS__)
#define _NewArray(type, co)			(os::IsMemoryExceptionEnabledInThread()? new type[co] : new (std::nothrow) type[co])

#define _DumpMemoryAllocations		{}

#endif

#define _SafeFree8AL(ptr)			{ _SafeFree8AL_ConstPtr(ptr); ptr = nullptr; }
#define _SafeFree32AL(ptr)			{ _SafeFree32AL_ConstPtr(ptr); ptr = nullptr; }
#define _SafeDel(ptr)				{ _SafeDel_ConstPtr(ptr); ptr = nullptr; }
#define _SafeDel_Untracked(ptr)		{ if(ptr){ delete ptr; ptr = nullptr; }}
#define _SafeDelArray(ptr)			{ _SafeDelArray_ConstPtr(ptr); ptr = nullptr; }

#define _EnlargeTo32AL(num)			((((num) + 0x7)&(~((SIZE_T)0x7))))
#define _Alloca32AL(sz)				(_EnlargeTo32AL((SIZE_T)alloca(sz + 4)))
#define _StackNew(type)				new (alloca(sizeof(type))) type

#ifndef PLATFORM_WIN
#define _alloca alloca
#endif

///////////////////////////////////////////////////////
// Misc inline templates
namespace rt
{

//INLFUNC void Randomize( DWORD seed = time(NULL) ){ srand(seed); }

template<typename T>
FORCEINL T MaxPower2Within(T within)
{
	if(within)
	{
		UINT ret=1;
		while(within!=1)
		{
			ret<<=1;
			within>>=1;
		}

		return ret;
	}
	else
		return 0;
}

template<typename T>
FORCEINL T* _CastToNonconst(const T * p)
{
	return (T*)((size_t)p);
}

} // namespace rt



/////////////////////////////////////////////////////
// switch little-endian/big-endian values, inplace
namespace rt
{
namespace _details
{	
	template<int sz>
	FORCEINL void _SwitchByteOrder(LPBYTE p)
	{	BYTE t = p[0];
		p[0] = p[sz-1];
		p[sz-1] = t;
		_SwitchByteOrder<sz-2>(p+1);
	}
	template<>
	FORCEINL void _SwitchByteOrder<1>(LPBYTE p){}
	template<>
	FORCEINL void _SwitchByteOrder<0>(LPBYTE p){}
} // namespace _details
} // namespace rt

namespace rt
{
	template<typename T>
	FORCEINL void SwitchByteOrder(T& x)
	{	_details::_SwitchByteOrder<sizeof(T)>((LPBYTE)&x);
	}
	template<typename T>
	FORCEINL T ConvertByteOrder(const T& x)
	{	T out = x;
		_details::_SwitchByteOrder<sizeof(T)>((LPBYTE)&out);
		return out;
	}
}

namespace rt
{
namespace _details
{	
	template<int sz>
	FORCEINL void _SwitchByteOrderTo(LPCBYTE f, LPBYTE t)
	{	*t = *f;
 		_SwitchByteOrderTo<sz-1>(f-1, t+1);
	}
	template<>
	FORCEINL void _SwitchByteOrderTo<0>(LPCBYTE f, LPBYTE t){}

	template<size_t sz, int block = sz>=8?8:(sz>=4?4:(sz>=2?2:1)), bool is_small = (sz<=128) >
	struct _CopyByteTo;
		template<size_t sz> struct _CopyByteTo<sz,8,true>
		{	static FORCEINL void Copy(LPCBYTE f, LPBYTE t)
			{	*((ULONGLONG*)t) = *((ULONGLONG*)f);
				_CopyByteTo<sz-8>::Copy(f+8, t+8);
		}	};
		template<size_t sz> struct _CopyByteTo<sz,4,true>
		{	static FORCEINL void Copy(LPCBYTE f, LPBYTE t)
			{	*((DWORD*)t) = *((DWORD*)f);
				_CopyByteTo<sz-4>::Copy(f+4, t+4);
		}	};
		template<size_t sz> struct _CopyByteTo<sz,2,true>
		{	static FORCEINL void Copy(LPCBYTE f, LPBYTE t)
			{	*((WORD*)t) = *((WORD*)f);
				_CopyByteTo<sz-2>::Copy(f+2, t+2);
		}	};
		template<> struct _CopyByteTo<1,1,true>
		{	static FORCEINL void Copy(LPCBYTE f, LPBYTE t)
			{	*((BYTE*)t) = *((BYTE*)f);
		}	};
		template<> struct _CopyByteTo<0, 1,true>
		{	static FORCEINL void Copy(LPCBYTE f, LPBYTE t){}
		};
		template<size_t sz, int block> struct _CopyByteTo<sz, block, false>
		{	static FORCEINL void Copy(LPCBYTE f, LPBYTE t){ memcpy(t, f, sz); }
		};
} // namespace _details
} // namespace rt

namespace rt
{
	template<typename T>
	FORCEINL void SwitchByteOrderTo(const T& from, T& to)
	{	_details::_SwitchByteOrderTo<sizeof(T)>(((LPCBYTE)&from) + sizeof(T) - 1, (LPBYTE)&to);
	}
	template<int SIZE>
	FORCEINL void SwitchByteOrderTo(LPCVOID from, LPVOID to)
	{	_details::_SwitchByteOrderTo<SIZE>(((LPCBYTE)from) + SIZE - 1, (LPBYTE)to);
	}
	template<typename T>
	FORCEINL void CopyByteTo(const T& from, T& to)
	{	_details::_CopyByteTo<sizeof(T)>::Copy((LPCBYTE)&from, (LPBYTE)&to);
	}
	template<int SIZE>
	FORCEINL void CopyByteTo(LPCVOID from, LPVOID to)
	{	_details::_CopyByteTo<SIZE>::Copy((LPCBYTE)from, (LPBYTE)to);
	}
	template<typename T>
	FORCEINL void CopyBits(T& x, T val, T mask)
	{
		ASSERT((val & (~mask)) == 0);
		x = (x&(~mask)) | val;
	}
} // namespace rt


namespace rt
{
FORCEINL WORD		BitSmudgeRight(WORD x) // (minimum power of 2 >= x) - 1
					{	UINT r = x | (x>>1);
						r |= r >> 2;	r |= r >> 4;	r |= r >> 8;
						return (WORD)r;
					}
FORCEINL UINT		BitSmudgeRight(UINT x) // (minimum power of 2 >= x) - 1
					{	UINT r = x | (x>>1);
						r |= r >> 2;	r |= r >> 4;	r |= r >> 8;	r |= r >> 16;
						return r;
					}
FORCEINL ULONGLONG	BitSmudgeRight(ULONGLONG x) // (minimum power of 2 >= x) - 1
					{	ULONGLONG r = x | (x>>1);
						r |= r >> 2;	r |= r >> 4;	r |= r >> 8;	r |= r >> 16;	r |= r >> 32;
						return (WORD)r;
					}
}


////////////////////////////////////////////////////////////////////////////
// Optimized Object Swap, UNSAFE if the object has self or member pointed pointers
// No additional temporary object, no ctor/dtor called
namespace rt
{
	namespace _details
	{
		template<UINT Len>
		FORCEINL void _Exchange_32AL(LPDWORD obj_a,LPDWORD obj_b)
		{
			DWORD t = *obj_a;
			*obj_a = *obj_b;
			*obj_b = t;

			rt::_details::_Exchange_32AL<Len-1>(obj_a+1,obj_b+1);
		}
		template<>
		FORCEINL void _Exchange_32AL<0>(LPDWORD,LPDWORD){} // dummy
	} // namespace _details
} // namespace rt

namespace rt
{
#pragma warning(disable: 4307) //warning C4307: '*' : integral constant overflow
template<typename T>
FORCEINL void Swap(T& a, T& b)
{	
	rt::_details::_Exchange_32AL<sizeof(T)/4> ((LPDWORD)&a, (LPDWORD)&b);
	if( (sizeof(T)&0x1) == 0x1 )
	{	DWORD t;
		t = ((LPBYTE)&a)[sizeof(T)-1];
		((LPBYTE)&a)[sizeof(T)-1] = ((LPBYTE)&b)[sizeof(T)-1];
		((LPBYTE)&b)[sizeof(T)-1] = (BYTE)t;
	}
	if( (sizeof(T)&0x2) == 0x2 )
	{	DWORD t;
		t = ((LPWORD)&a)[sizeof(T)/2-1];
		((LPWORD)&a)[sizeof(T)/2-1] = ((LPWORD)&b)[sizeof(T)/2-1];
		((LPWORD)&b)[sizeof(T)/2-1] = (WORD)t;
	}
}
#pragma warning(default: 4307) //warning C4307: '*' : integral constant overflow

} // namespace rt


///////////////////////////////////////////////////////
// Advanced Bits Operations
namespace rt
{
	// count leading zeros
	template<typename T> INLFUNC UINT LeadingZeroBits(T x);
#if defined(PLATFORM_WIN)
	template<> INLFUNC UINT LeadingZeroBits(WORD x) { DWORD index; return _BitScanReverse(&index, (DWORD)x) ? 16U - 1U - (UINT)index : 16U; }
	template<> INLFUNC UINT LeadingZeroBits(DWORD x) { DWORD index; return _BitScanReverse(&index, x) ? 32U - 1U - (UINT)index : 32U; }
	#if defined(PLATFORM_64BIT)
	template<> INLFUNC UINT LeadingZeroBits(ULONGLONG x) { DWORD index; return _BitScanReverse64(&index, x) ? 64U - 1U - (UINT)index : 64U; }
	#endif
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
	template<> INLFUNC UINT LeadingZeroBits(WORD x) { if (0 == x) return 16; return (UINT)__builtin_clz((DWORD)x) - 16; }
	template<> INLFUNC UINT LeadingZeroBits(DWORD x) { if (0 == x) return 32; return (UINT)__builtin_clz(x); }
	template<> INLFUNC UINT LeadingZeroBits(ULONGLONG x) { if (0 == x) return 64; return (UINT)__builtin_clzll(x); }
#else
#pragma message ("Advanced Bits Operations count-leading-zeros is not available")
#endif	

	// count trailing zeros
	template<typename T> INLFUNC UINT TrailingZeroBits(T x);
#if defined(PLATFORM_WIN)
	template<> INLFUNC UINT TrailingZeroBits(WORD x) { DWORD index; return _BitScanForward(&index, (DWORD)x) ? (UINT)index : 16U; }
	template<> INLFUNC UINT TrailingZeroBits(DWORD x) { DWORD index; return _BitScanForward(&index, x) ? (UINT)index : 32U; }
	#if defined(PLATFORM_64BIT)
	template<> INLFUNC UINT TrailingZeroBits(ULONGLONG x) { DWORD index; return _BitScanForward64(&index, x) ? (UINT)index : 64U; }
	#endif
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
	template<> INLFUNC UINT TrailingZeroBits(WORD x) { if (0 == x) return 16; return (UINT)__builtin_ctz((DWORD)x); }
	template<> INLFUNC UINT TrailingZeroBits(DWORD x) { if (0 == x) return 32; return (UINT)__builtin_ctz(x); }
	template<> INLFUNC UINT TrailingZeroBits(ULONGLONG x) { if (0 == x) return 64; return (UINT)__builtin_ctzll(x); }
#else
#pragma message ("Advanced Bits Operations count-trailing-zeros is not available")
#endif	

	// count 1s
	template<typename T> INLFUNC UINT NonzeroBits(T x);
#if defined(PLATFORM_WIN)
	template<> INLFUNC UINT NonzeroBits(WORD x) { return (UINT)__popcnt16(x); }
	template<> INLFUNC UINT NonzeroBits(DWORD x) { return (UINT)__popcnt(x); }
	#if defined(PLATFORM_64BIT)
	template<> INLFUNC UINT NonzeroBits(ULONGLONG x) { return (UINT)__popcnt64(x); }
	#endif
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
	template<> INLFUNC UINT NonzeroBits(WORD x) { return (UINT)__builtin_popcount((DWORD)x); }
	template<> INLFUNC UINT NonzeroBits(DWORD x) { return (UINT)__builtin_popcount(x); }
	template<> INLFUNC UINT NonzeroBits(ULONGLONG x) { return (UINT)__builtin_popcountll(x); }
#else
#pragma message ("Advanced Bits Operations count-pop-bits is not available")
#endif

	// swap byte order
	template<typename T> INLFUNC T ByteOrderSwap(T x);
#if defined(PLATFORM_WIN)
	template<> INLFUNC WORD ByteOrderSwap(WORD x) { return _byteswap_ushort(x); }
	template<> INLFUNC DWORD ByteOrderSwap(DWORD x) { return _byteswap_ulong(x); }
	#if defined(PLATFORM_64BIT)
	template<> INLFUNC ULONGLONG ByteOrderSwap(ULONGLONG x) { return _byteswap_uint64(x); }
	#endif
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
	template<> INLFUNC WORD ByteOrderSwap(WORD x) { return __builtin_bswap16(x); }
	template<> INLFUNC DWORD ByteOrderSwap(DWORD x) { return __builtin_bswap32(x); }
	template<> INLFUNC ULONGLONG ByteOrderSwap(ULONGLONG x) { return __builtin_bswap64(x); }
#else
#pragma message ("Advanced Bits Operations byte-order-swap is not available")
#endif	

	// sub_borrow
	template<typename T> INLFUNC BYTE SubBorrow(BYTE, T, T, T*);
#if defined(PLATFORM_WIN) || defined(PLATFORM_MAC)
	template<> INLFUNC BYTE SubBorrow(BYTE b_in, UINT a, UINT b, UINT* out) { return _subborrow_u32(b_in, a, b, out); }
	#if defined(PLATFORM_64BIT)
	template<> INLFUNC BYTE SubBorrow(BYTE b_in, ULONGLONG a, ULONGLONG b, ULONGLONG* out) { return _subborrow_u64(b_in, a, b, out); }
	#endif
#elif defined(PLATFORM_LINUX)
	template<> INLFUNC BYTE SubBorrow(BYTE b_in, UINT a, UINT b, UINT* out) { return __builtin_ia32_sbb_u32(b_in, a, b, out); }
	template<> INLFUNC BYTE SubBorrow(BYTE b_in, ULONGLONG a, ULONGLONG b, ULONGLONG* out) { return __builtin_ia32_sbb_u64(b_in, a, b, out); }
#else
#pragma message ("SubBorrow is not available")
#endif
}

namespace rt
{

namespace _details
{
template<class T, typename t_Val, class t_GetKey>	// T should have operator [], and the element should have operator <
INLFUNC SIZE_T _LowerBound(const T& arr, SIZE_T len, const t_Val &key)	// first item equal to or greater than that of a specified key
{
	if(len == 0 || !(t_GetKey::Key(arr[0]) < key))return 0;
	if( !(t_GetKey::Key(arr[len - 1]) < key) ){}
	else{ return len; }

	SIZE_T ret = len - 1;
	SIZE_T L = 0;
	while ( L + 1 < ret )
	{	SIZE_T mid = (L + ret) >> 1;
		if ( t_GetKey::Key(arr[mid]) < key )
			L = mid;
		else
			ret = mid;					
	}
	return ret;
}

template<class T, typename t_Val, class t_GetKey>
INLFUNC SIZE_T _BinarySearch(const T& arr, SIZE_T len, const t_Val &key)	// first item equal to key, or length of arr
{
	SIZE_T r = _LowerBound<T,t_Val,t_GetKey>(arr,len,key);
	if(r<len && key < t_GetKey::Key(arr[r]))return len;
	return r;
}

struct _TrivialGetKey
{	template<typename T> static FORCEINL T& Key(T& x){ return x; }
};

} // namespace _details

template<class t_GetKey, class T, typename t_Val>	// T should have operator [], and the element should have operator <
FORCEINL SIZE_T LowerBound(const T& arr, SIZE_T len, const t_Val &key)	// first item equal to or greater than that of a specified key
{	return _details::_LowerBound<T,t_Val,t_GetKey>(arr,len,key);
}

template<class t_GetKey, class T, typename t_Val>
FORCEINL SIZE_T BinarySearch(const T& arr, SIZE_T len, const t_Val &key)	// first item equal to key, or length of arr
{	return _details::_BinarySearch<T,t_Val,t_GetKey>(arr,len,key);
}

template<class T, typename t_Val>	// T should have operator [], and the element should have operator <
FORCEINL SIZE_T LowerBound(const T& arr, SIZE_T len, const t_Val &key)	// first item equal to or greater than that of a specified key
{	return _details::_LowerBound<T,t_Val,_details::_TrivialGetKey>(arr,len,key);
}

template<class T, typename t_Val>
FORCEINL SIZE_T BinarySearch(const T& arr, SIZE_T len, const t_Val &key)	// first item equal to key, or length of arr
{	return _details::_BinarySearch<T,t_Val,_details::_TrivialGetKey>(arr,len,key);
}



template<class T>
class ObjRef
{	
protected:
	T*	_p;
public:
	typedef T	t_Obj;
	INLFUNC ObjRef(T*x):_p(x){}
	INLFUNC ObjRef(){ Empty(); }
	INLFUNC T* operator ->(){ return _p; }
	INLFUNC const T* operator ->() const { return _p; }
	INLFUNC operator T& () { return *_p; }
	INLFUNC operator const T& () const { return *_p; }
	INLFUNC operator T* () { return _p; }
	INLFUNC operator const T* () const { return _p; }

	INLFUNC T* operator = (T*x){ _p = x; return x; }

	INLFUNC bool	IsEmpty() const { return _p == nullptr; }  
	INLFUNC void	Empty(){ _p=nullptr; }
	INLFUNC void	SafeDel(){ _SafeDel(_p); }
	INLFUNC void	SafeRelease(){ _SafeRelease(_p); }
};

template<typename T>
struct Singleton
{
	static T& Get(){ static T ret; return ret; }
};

template<UINT INIT_INTERVAL>
struct FrequencyDivision
{
	int	 _TickOffset;
	UINT _Interval;
public:
	FrequencyDivision(){ Reset(); }
	void Reset(){ _TickOffset = -1; _Interval = INIT_INTERVAL; }
	void SetInterval(UINT inv){ _Interval = inv; }
	bool Hit(UINT tick, bool no_phase_align = false)
	{	if(no_phase_align)
		{	return (tick-_TickOffset)%_Interval == 0;
		}
		else
		{	if(_TickOffset < 0)
			{	_TickOffset = tick;
				return true; 
			}
			return (tick-_TickOffset)%_Interval == 0;
		}
	}
	bool operator ()(UINT tick){ return Hit(tick); }
};

template<typename T>
class ObjectPlaceHolder
{
	BYTE	_Place[sizeof(T)];
	bool	_bInit;
	void	_Copy(const ObjectPlaceHolder<T>& x){ if(x._bInit){ new (_Place) T(x); _bInit = true; }else{ _bInit = false; } }

public:
	ObjectPlaceHolder(){ _bInit = false; }
	ObjectPlaceHolder(const T& x){ new (_Place) T(x); _bInit = true; }
	ObjectPlaceHolder(const ObjectPlaceHolder<T>& x){ _Copy(x); }

	const T& operator = (const T& x){ Term(); new (_Place) T(x); _bInit = true; return x; }
	const ObjectPlaceHolder<T>& operator = (const ObjectPlaceHolder<T>& x){ Term(); _Copy(x); return x; }

	~ObjectPlaceHolder(){ Term(); }
	void Term(){ if(_bInit){ _bInit = false; ((T*)_Place)->~T(); } }

	ObjectPlaceHolder<T>&	Init(){ ASSERT(!_bInit); new (_Place) T; _bInit = true; return *this; }
	ObjectPlaceHolder<T>&	Reinit(){ Term(); new (_Place) T; _bInit = true; return *this; }

	ObjectPlaceHolder<T>&	Init(const ObjectPlaceHolder<T>& x){ ASSERT(!_bInit); _Copy(x); return *this; }
	ObjectPlaceHolder<T>&	Reinit(const ObjectPlaceHolder<T>& x){ Term(); _Copy(x); return *this; }

	template<typename... Params>
	ObjectPlaceHolder<T>&	Init(Params&&... args){ ASSERT(!_bInit); new (_Place) T(std::forward<Params>(args)...); _bInit = true; return *this; }
	template<typename... Params>
	ObjectPlaceHolder<T>&	Reinit(Params&&... args){ Term(); new (_Place) T(std::forward<Params>(args)...); _bInit = true; return *this; }

	T&						Object(){ return *(T*)_Place; }
	const T&				Object() const { return *(const T*)_Place; }
	bool					IsInitialized() const { return _bInit; }

	operator T&			(){ ASSERT(_bInit); return *(T*)_Place; }
	operator const T&	() const { ASSERT(_bInit); return *(const T*)_Place; }
	operator T*			(){ ASSERT(_bInit); return (T*)_Place; }
	operator const T*	() const { ASSERT(_bInit); return (const T*)_Place; }

	T* operator ->		(){ ASSERT(_bInit); return (T*)_Place; }
	const T* operator ->() const { ASSERT(_bInit); return (const T*)_Place; }
};


// simple pseudo-random number generator invented by George Marsaglia.
// http://en.wikipedia.org/wiki/Random_number_generation
class Randomizer
{
	UINT m_z, m_w;
public:
	INLFUNC Randomizer(UINT seed = 17)
	{	if(seed==0 || seed==0x464fffff || (seed*seed)==0x9068ffff)seed+=0x52897528;
		m_w = seed;
		m_z = seed*seed;
	}
	FORCEINL UINT GetNext()
	{	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
		m_w = 18000 * (m_w & 65535) + (m_w >> 16);
		return (m_z << 16) + m_w;
	}
	template<typename T>
	FORCEINL void Randomize(T& pod)
	{	Randomize(&pod, sizeof(T));
	}
	FORCEINL void Randomize(LPVOID p_in, SIZE_T sz)
	{	UINT* p = (UINT*)p_in;
		for(UINT i=0;i<sz/sizeof(UINT);i++)
			p[i] = GetNext();
		if(sz&3)
		{	BYTE num[4];
			*((UINT*)num) = GetNext();
			LPBYTE t = ((LPBYTE)p_in) + (sz&(~3));
			LPBYTE tend = ((LPBYTE)p_in) + sz;
			for(SIZE_T i=0;t<tend;t++, i++)
				*t = num[i];
		}
	}
	FORCEINL operator UINT(){ return GetNext(); }
};

} // namespace rt
////////////////////////////////////////////////////////////////////////////

