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

#ifndef __HUFF_DECODE_H__
#define __HUFF_DECODE_H__

#ifdef _FULLDIAG
#pragma message("  Huff_Decode.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif
#ifndef __DECODE_BUFFER_H__
#include "buffer/decode_buffer.h"
#endif
#ifndef __TRACE_H__
#include "interface_manager/trace.h"
#endif




/* //////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(IJLERR,DecodeHuffman8x8,(
  STATE*         pState,
  HUFFMAN_TABLE* pDcTable,
  HUFFMAN_TABLE* pAcTable,
  Ipp16s*        pLastDC,
  Ipp16s*        pDst));

OWNAPI(IJLERR,Decode_Block_Prog_DC,(
  STATE*         pState,
  HUFFMAN_TABLE* pDcTable,
  Ipp16s*        pDst,
  Ipp16s*        pLastDC,
  int            Al));

OWNAPI(IJLERR,Decode_Block_Prog_DC_SA,(
  STATE*         pState,
  Ipp16s*        pDst,
  int            Al));

OWNAPI(IJLERR,Decode_Block_Prog_AC,(
  STATE*         pState,
  Ipp16s*        pDst,
  HUFFMAN_TABLE* pAcTable,
  int            Ss,
  int            Se,
  int            Al));

OWNAPI(IJLERR,Decode_Block_Prog_AC_SA,(
  STATE*         pState,
  Ipp16s*        pDst,
  HUFFMAN_TABLE* pAcTable,
  int            Ss,
  int            Se,
  int            Al));


#endif /* __HUFF_DECODE_H__ */


