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

#ifndef __DECODER_PARSER_H__
#define __DECODER_PARSER_H__

#ifdef _FULLDIAG
#pragma message("  Decoder_Parser.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif
#ifndef __DECODE_BUFFER_H__
#include "decode_buffer.h"
#endif
#ifndef __HUFF_DECODE_H__
#include "huff_decode.h"
#endif
#ifndef __SCAN_DECODER_H__
#include "scan_decoder.h"
#endif
#ifndef __TABLES_H__
#include "tables.h"
#endif
#ifndef __IPP_IDCT_H__
#include "ipp_idct.h"
#endif




/* ///////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(IJLERR,DP_Skip,(
  STATE* pState));

OWNAPI(IJLERR,DP_Get_Next_Marker,(
  STATE*      pState,
  IJL_MARKER* pMarker));

OWNAPI(IJLERR,DP_Parse_APP,(
  STATE* pState));

OWNAPI(IJLERR,DP_Parse_DRI,(
  STATE* pState,
  int*   pRstInterval));

OWNAPI(IJLERR,DP_Parse_SOF,(
  STATE*           pState,
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,DP_Parse_DQT,(
  STATE*           pState,
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,DP_Parse_APP14,(
  int*    pAdobeFound,
  Ipp16u* pAdobeVersion,
  Ipp16u* pAdobeFlags0,
  Ipp16u* pAdobeFlags1,
  int*    pAdobeXform,
  STATE*  pState));

OWNAPI(IJLERR,DP_Parse_APP0,(
  STATE*           pState,
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,DP_Parse_COM,(
  STATE*  pState,
  Ipp8u*  pComment,
  Ipp16u* pCommentSize));

OWNAPI(IJLERR,DP_Parse_DHT,(
  STATE*           pState,
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,DP_Parse_SOS,(
  STATE*           pState,
  FRAME*           pFrame,
  SCAN*            pScan,
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,Get_Scan,(
  STATE*           pState,
  SCAN*            pScan,
  JPEG_PROPERTIES* jprops));

OWNAPI(IJLERR,DP_ParseBitstream,(
  JPEG_PROPERTIES* jprops));


#endif /* __DECODER_PARSER_H__ */

