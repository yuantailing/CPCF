/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 1998-2006 Intel Corporation. All Rights Reserved.
//
//  Intel(R) Integrated Performance Primitives Intel(R) JPEG Library -
//        Intel(R) IPP Version for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel(R) Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel(R) IPP product
//  installation for more information.
//
//  JPEG is an international standard promoted by ISO/IEC and other organizations.
//  Implementations of these standards, or the standard enabled platforms may
//  require licenses from various entities, including Intel Corporation.
//
*/

#ifndef __OWN_H__
#define __OWN_H__

#ifdef _FULLDIAG
#pragma message("  own.h")
#endif




#if defined (__IJL_H__)
#error Only one of "ijl.h" or "own.h" may be used
#endif


#ifndef IJL_ALL_WARNINGS
#if defined (_WIN32)
#if _MSC_VER >= 1000

// nonstandard extension used : nameless struct/union
#pragma warning(disable : 4201)
// nonstandard extension used : bit field types other than int
#pragma warning(disable : 4214)
// unreferenced inline function has been removed
#pragma warning(disable : 4514)
// named type definition in parentheses
#pragma warning(disable : 4115)
// conditional expression is constant
#pragma warning(disable:4127)


#endif // _MSC_VER >= 1000
#endif // _WIN32
#endif // IJL_ALL_WARNINGS



#include <stdio.h>     // !vd for _vsnprintf
#include <stdlib.h>
#include <string.h>    // !vd for _strset in trace
#if defined (_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif // _WIN32

//#include "inc/ippdefs.h"
//#include "inc/ippcore.h"
//#include "inc/ipps.h"
//#include "inc/ippi.h"
//#include "inc/ippj.h"


#if !defined (_WIN32)
#include <setjmp.h>
#include <signal.h>
#endif

#ifndef IJL_STDCALL
#define IJL_STDCALL __STDCALL
#endif

#ifndef IJL_CDECL
#define IJL_CDECL __CDECL
#endif


#ifdef IJL_MSEXPORTS

  #define IJLAPI(type,name,arg) \
    extern __declspec(dllexport) type IJL_STDCALL name arg

#else

  #define IJLAPI(type,name,arg) \
    extern type IJL_STDCALL name arg

#endif


// NOTE: include ijl.h after defining IJLAPI macro for export functions
#include "ijl.h"




#ifdef IJL_MSEXPORTS

  #define IJLFUN(type) \
            extern __declspec(dllexport) type IJL_STDCALL
  #define OWNFUN(type) \
            extern __declspec(dllexport) type IJL_CDECL
  #define OWNAPI(type,name,arg)  \
            extern __declspec(dllexport) type IJL_CDECL name arg

#else // IJL_LIB

  #define IJLFUN(type) \
            extern type IJL_STDCALL
  #define OWNFUN(type) \
            extern type IJL_CDECL
  #define OWNAPI(type,name,arg)  \
            extern type IJL_CDECL name arg

#endif // IJL_DLL


#define LOCVAR(type) \
            static type
#define LOCAPI(type,name,arg)  \
            static type IJL_CDECL name arg
#define LOCFUN(type) \
            static type IJL_CDECL


#if defined (_WIN32)
#define IJL_INLINE __inline
#else
#define IJL_INLINE __inline__
#endif


#if !defined (_WIN32)
/*
typedef unsigned int DWORD;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned char BYTE;
//typedef unsigned int BOOL;
typedef void* LPVOID;
typedef unsigned int* LPDWORD;
typedef const char* LPCSTR;
typedef char* LPSTR;

typedef struct tagPOINT
{
  long x;
  long y;
} POINT, *PPOINT;

#define FALSE 0
#define TRUE 1
*/
#define GENERIC_READ  1
#define GENERIC_WRITE 2
#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
//#define INVALID_HANDLE_VALUE 0xffffffff
#define CREATE_ALWAYS 0
#define OPEN_EXISTING 1
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define UNREFERENCED_PARAMETER(a) (void)(a)
#define TEXT(str) str
#endif


#define CLIP(i) ((i) > 255 ? 255 : ((i) < 0 ? 0 : (i)))

#define CPU_CACHE_LINE      32
#define DCTSIZE             8
#define DCTSIZE2            64
#define IJL_OPAQUE          0xFF
#define MAX_COMMENTS_SIZE   65531
#define MAX_COMP_PER_FRAME  255
#define MAX_COMP_PER_SCAN   4
#define MAX_BLKS_PER_MCU    10
#define MAX_MCU_SIZE        (DCTSIZE2*MAX_BLKS_PER_MCU)
#define MIN_DCT_VAL         (-1023)
#define MAX_DCT_VAL         ( 1023)
#define MAX_MCU_ROWS        8192

#define JFXX_THUMBNAILS_JPEG  0x10
#define JFXX_THUMBNAILS_GRAY  0x11
#define JFXX_THUMBNAILS_RGB   0x13


#define OWN_ALIGN_PTR(ptr,align) \
  (ippAlignPtr(ptr,align))


#if defined (_WIN32)

#define EXC_INIT()
#define TRY_BEGIN   __try {
#define TRY_END     }
#define CATCH_BEGIN __except(EXCEPTION_EXECUTE_HANDLER) {
#define CATCH_END   }

#else

typedef void (*sighandler_t)(int);

#define EXC_INIT() \
  static jmp_buf sJmpBuf; \
  static void IJL_CDECL own_sig_fpe_ill_segv(int code) {longjmp(sJmpBuf,-1);}

#define TRY_BEGIN   if(0 == setjmp(sJmpBuf)) {
#define TRY_END     }
#define CATCH_BEGIN else {
#define CATCH_END   }

#endif


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

typedef void (IJL_CDECL *PFN_CC_AND_SS_MCU)(
  JPEG_PROPERTIES*  src,
  int    curxMCU,
  int    curyMCU);

typedef void (IJL_CDECL *PFN_US_AND_CC_MCU)(
  JPEG_PROPERTIES*  src,
  int    curxMCU,
  int    curyMCU);


/*D*
////////////////////////////////////////////////////////////////////////////
//  Name
//    IJL_OPEN_MODE
//
//  Purpose
//    Possible flags for CreateFile function
//
//  Fields
//
//    OPEN_READ       Indicates generic read access.
//
//    OPEN_WRITE      Indicates generic write access.
//
//    OPEN_READ_WRITE read-write access
//
////////////////////////////////////////////////////////////////////////////
*D*/

typedef enum
{
  OPEN_READ       = GENERIC_READ,
  OPEN_WRITE      = GENERIC_WRITE,
  OPEN_READ_WRITE = (GENERIC_READ | GENERIC_WRITE)
} IJL_OPEN_MODE;




/*D*
////////////////////////////////////////////////////////////////////////////
//  Name
//    IJL_MARKER
//
//  Purpose
//    Possible values for JPEG markers
//
////////////////////////////////////////////////////////////////////////////
*D*/

typedef enum
{
  MARKER_NONE   = 0,
  MARKER_EOF    = 0x100,
  MARKER_SOF0   = 0xc0,
  MARKER_SOF1   = 0xc1,
  MARKER_SOF2   = 0xc2,
  MARKER_SOF3   = 0xc3,
  MARKER_SOF5   = 0xc5,
  MARKER_SOF6   = 0xc6,
  MARKER_SOF7   = 0xc7,
  MARKER_SOF8   = 0xc8,
  MARKER_SOF9   = 0xc9,
  MARKER_SOFA   = 0xca,
  MARKER_SOFB   = 0xcb,
  MARKER_SOFD   = 0xcd,
  MARKER_SOFE   = 0xce,
  MARKER_SOFF   = 0xcf,

  MARKER_DHT    = 0xc4,
  MARKER_DAC    = 0xcc,

  MARKER_RST0   = 0xd0,
  MARKER_RST1   = 0xd1,
  MARKER_RST2   = 0xd2,
  MARKER_RST3   = 0xd3,
  MARKER_RST4   = 0xd4,
  MARKER_RST5   = 0xd5,
  MARKER_RST6   = 0xd6,
  MARKER_RST7   = 0xd7,

  MARKER_SOI    = 0xd8,
  MARKER_EOI    = 0xd9,
  MARKER_SOS    = 0xda,
  MARKER_DQT    = 0xdb,
  MARKER_DNL    = 0xdc,
  MARKER_DRI    = 0xdd,
  MARKER_DHP    = 0xde,
  MARKER_EXP    = 0xdf,

  MARKER_APP0   = 0xe0,  /* JFIF (hopefully) application extension.      */
  MARKER_APP1   = 0xe1,  /* Unsupported application specific extensions. */
  MARKER_APP2   = 0xe2,
  MARKER_APP3   = 0xe3,
  MARKER_APP4   = 0xe4,
  MARKER_APP5   = 0xe5,
  MARKER_APP6   = 0xe6,
  MARKER_APP7   = 0xe7,
  MARKER_APP8   = 0xe8,
  MARKER_APP9   = 0xe9,
  MARKER_APP10  = 0xea,
  MARKER_APP11  = 0xeb,
  MARKER_APP12  = 0xec,
  MARKER_APP13  = 0xed,
  MARKER_APP14  = 0xee,  /* Adobe-specific application extension. */

  MARKER_JPG1   = 0xf0,  /* From 0xf0 to 0xfd. */
  MARKER_JPG2   = 0xf1,
  MARKER_JPG3   = 0xf2,
  MARKER_JPG4   = 0xf3,
  MARKER_JPG5   = 0xf4,
  MARKER_JPG6   = 0xf5,
  MARKER_JPG7   = 0xf6,
  MARKER_JPG8   = 0xf7,
  MARKER_JPG9   = 0xf8,
  MARKER_JPG10  = 0xf9,
  MARKER_JPG11  = 0xfa,
  MARKER_JPG12  = 0xfb,
  MARKER_JPG13  = 0xfc,
  MARKER_JPG14  = 0xfd,
  MARKER_COM    = 0xfe

} IJL_MARKER;




/*D*
////////////////////////////////////////////////////////////////////////////
//  Name
//    IJL_CONTEXT
//
//  Purpose
//    To store global variables in thread safe manner
//
////////////////////////////////////////////////////////////////////////////
*D*/

typedef struct _IJL_CONTEXT
{
  int                scan_length;

  void*              pHuffStateBuf;
  Ipp16s*            pDCTCoefBuffer;

  PFN_CC_AND_SS_MCU  __g_cc_and_ss_mcu;
  PFN_US_AND_CC_MCU  __g_us_and_cc_mcu;

} IJL_CONTEXT;


#endif // __OWN_H__
