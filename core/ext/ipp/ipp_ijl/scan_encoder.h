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

#ifndef __SCAN_ENCODER_H__
#define __SCAN_ENCODER_H__

#ifdef _FULLDIAG
#pragma message("  Scan_Encoder.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif
#ifndef __TABLES_H__
#include "tables/tables.h"
#ifndef __IPP_FDCT_H__
#include "dct/ipp_fdct.h"
#endif
#endif
#ifndef __HUFF_ENCODE_H__
#include "entropy_codec/huff_encode.h"
#endif
#ifndef __ENCODER_PARSER_H__
#include "parser/encoder_parser.h"
#endif




/* ///////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(IJLERR,EncodeScanBaseline,(
  JPEG_PROPERTIES* jprops,
  SCAN*            pScan,
  STATE*           pState));

OWNAPI(IJLERR,EncodeScanProgressive,(
  JPEG_PROPERTIES* jprops,
  SCAN*            pScan,
  STATE*           pState));

OWNAPI(IJLERR,Fill_coeffs_buffer,(
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,Gather_statistics,(
  JPEG_PROPERTIES* jprops,
  SCAN*            pScan,
  int              FREQ[2][256],
  STATE*           pState));

#endif /* __SCAN_ENCODER_H__ */


