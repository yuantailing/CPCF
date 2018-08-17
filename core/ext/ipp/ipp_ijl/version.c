/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 1998-2006 Intel Corporation. All Rights Reserved.
//
//  Intel?Integrated Performance Primitives Intel?JPEG Library -
//        Intel?IPP Version for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel?Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel?IPP product
//  installation for more information.
//
//  JPEG is an international standard promoted by ISO/IEC and other organizations.
//  Implementations of these standards, or the standard enabled platforms may
//  require licenses from various entities, including Intel Corporation.
//
*/

#include "precomp.h"
#ifndef __VERSION_H__
#include "interface_manager/version.h"
#endif



/* NOTE:                                */
/*   MAJOR, MINOR, BUILD                */
/*   and STR macro defined in version.h */
/*   Change of this values has global   */
/*   effect on the library.             */

static IJLibVersion libVersion =
{
  MAJOR,

  MINOR,

  BUILD,

  /* library name */
#if defined (_WIN32)
#if defined (IJL_DLL)
  #if defined (_DEBUG)
    "ijl" STR(MAJOR) STR(MINOR1) "d.dll",
  #elif defined(NDEBUG) && defined(ENABLE_TIMING)
    "ijl" STR(MAJOR) STR(MINOR1) "t.dll",
  #else
    "ijl" STR(MAJOR) STR(MINOR1) ".dll",
  #endif
#else
  #if defined (_DEBUG)
    "ijl" STR(MAJOR) STR(MINOR1) "dl.lib",
  #elif defined(NDEBUG) && defined(ENABLE_TIMING)
    "ijl" STR(MAJOR) STR(MINOR1) "tl.lib",
  #else
    "ijl" STR(MAJOR) STR(MINOR1) "l.lib",
  #endif
#endif
#else /* _WIN32 */
#if defined (IJL_DLL)
  #if defined (_DEBUG)
    "libijl" STR(MAJOR) STR(MINOR1) "d.so",
  #elif defined(NDEBUG) && defined(ENABLE_TIMING)
    "libijl" STR(MAJOR) STR(MINOR1) "t.so",
  #else
    "libijl" STR(MAJOR) STR(MINOR1) ".so",
  #endif
#else
  #if defined (_DEBUG)
    "ijl" STR(MAJOR) STR(MINOR1) "dl.a",
  #elif defined(NDEBUG) && defined(ENABLE_TIMING)
    "ijl" STR(MAJOR) STR(MINOR1) "tl.a",
  #else
    "ijl" STR(MAJOR) STR(MINOR1) "l.a",
  #endif
#endif
#endif
  /* version */
  STR(MAJOR.MINOR.BUILD),

  /* internal version */
  STR([MAJOR.MINOR.BUILD.MAGIC]),
  /* build date */
  __DATE__,

  /* calling conventions */
#if defined(_WIN32)
#if defined(IJL_DLL)
  "DLL"
#else
  "Microsoft*"
#endif
#else /* _WIN32 */
#if defined(IJL_DLL)
  "Unix shared"
#else
  "Unix static"
#endif
#endif
};



OWNFUN(const IJLibVersion*) ownGetLibVersion(void)
{
  return &libVersion;
} /* ownGetLibVersion() */

