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

#ifndef __CC_SS_DECODER_H__
#define __CC_SS_DECODER_H__

#ifdef _FULLDIAG
#pragma message("  CC_SS_Decoder.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif




/* ///////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(void,Y_111_to_Y_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_111_to_RGB_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_111_to_BGR_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_422_to_RGB_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_422_to_BGR_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_411_to_RGB_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_411_to_BGR_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_111_to_RGBA_FPX_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_422_to_RGBA_FPX_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_411_to_RGBA_FPX_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCrA_FPX_1111_to_RGBA_FPX_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCrA_FPX_4224_to_RGBA_FPX_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCrA_FPX_4114_to_RGBA_FPX_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_111_to_OTHER4_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_1111_to_OTHER_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_111_to_OTHER_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_411_to_OTHER_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,OTHER_422_to_OTHER_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_422_to_YCbYCr_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,US_CC_General_YCbCr_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,CC_General_YCbCr_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));


OWNAPI(void,Output_Interleave_General_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,US_General_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,US_General_P_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_422_to_YCbYCr_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,YCbCr_422_to_YCbYCr_NI_MCU,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,Put_MCU_To_YCbYCr,(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU));

OWNAPI(void,US_And_CC_MCU_Scaled,(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  SCAN*            scan,
  int              curxMCU,
  int              curyMCU,
  int              scaledim,
  int              iDCT_instead_of_US));

OWNAPI(void,CConvert_Image,(
  JPEG_PROPERTIES* jprops));


#endif /* __CC_SS_DECODER_H__ */

