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

#ifndef __FRAME_ENCODER_H__
#define __FRAME_ENCODER_H__

#ifdef _FULLDIAG
#pragma message("  Frame_Encoder.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif
#ifndef __ENCODE_BUFFER_H__
#include "encode_buffer.h"
#endif
#ifndef __ENCODER_PARSER_H__
#include "encoder_parser.h"
#endif
#ifndef __TABLES_H__
#include "tables.h"
#endif
#ifndef __SCAN_ENCODER_H__
#include "scan_encoder.h"
#endif
#ifndef __IPP_FDCT_H__
#include "ipp_fdct.h"
#endif




/* ///////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(IJLERR,ownEncoderWriteJPEGTables,(
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,Encode_Frame_Init,(
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,Encode_Frame_Baseline,(
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,Encode_Frame_Progressive,(
  JPEG_PROPERTIES* jprops));

#endif /* __FRAME_ENCODER_H__ */


