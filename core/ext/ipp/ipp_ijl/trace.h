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

#ifndef __TRACE_H__
#define __TRACE_H__

#ifdef _FULLDIAG
#pragma message("  trace.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif




/* TraceFlags ... */
extern const unsigned int trBEFORE;
extern const unsigned int trAFTER;
extern const unsigned int trINFO;
extern const unsigned int trWARN;
extern const unsigned int trERROR;
extern const unsigned int trMEMORY;
extern const unsigned int trCALL;


#ifdef _DIAG

  #define TRACE0(mode,msg)
  #define TRACE1(mode,msg,p1)
  #define TRACE2(mode,msg,p1,p2)
  #define TRACE3(mode,msg,p1,p2,p3)
  #define TRACE4(mode,msg,p1,p2,p3,p4)

#else

  #define TRACE0(mode,msg)
  #define TRACE1(mode,msg,p1)
  #define TRACE2(mode,msg,p1,p2)
  #define TRACE3(mode,msg,p1,p2,p3)
  #define TRACE4(mode,msg,p1,p2,p3,p4)

#endif


#endif /* __TRACE_H__ */

