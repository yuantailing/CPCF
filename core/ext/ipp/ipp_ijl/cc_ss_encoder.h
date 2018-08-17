/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 1998-2006 Intel Corporation. All Rights Reserved.
//
//  Intel® Integrated Performance Primitives Intel® JPEG Library -
//        Intel® IPP Version for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel® Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel® IPP product
//  installation for more information.
//
//  JPEG is an international standard promoted by ISO/IEC and other organizations.
//  Implementations of these standards, or the standard enabled platforms may
//  require licenses from various entities, including Intel Corporation.
//
*/

#ifndef __CC_SS_ENCODER_H__
#define __CC_SS_ENCODER_H__

#ifdef _FULLDIAG
#pragma message("  CC_SS_Encoder.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif




/* ///////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(void,RGB_to_YCbCr_111_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,RGB_to_YCbCr_422_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,RGB_to_YCbCr_411_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,BGR_to_YCbCr_111_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,BGR_to_YCbCr_422_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,BGR_to_YCbCr_411_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,RGBA_FPX_to_YCbCrA_FPX_1111_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,RGBA_FPX_to_YCbCrA_FPX_4224_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,RGBA_FPX_to_YCbCrA_FPX_4114_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_to_OTHER_1111_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_to_OTHER_111_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_to_OTHER_422_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_to_OTHER_411_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbYCr_to_YCbCr_422_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,Y_to_Y_111_MCU,(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU));


OWNAPI(void,CC_SS_RGB_to_YCbCr_General_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,CC_RGB_to_YCbCr_General_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,SS_General_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,Input_Interleave_General_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));


#endif /* __CC_SS_ENCODER_H__ */

