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

#ifndef __HUFF_ENCODE_H__
#define __HUFF_ENCODE_H__

#ifdef _FULLDIAG
#pragma message("  Huff_Encode.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif
#ifndef __FILEIO_H__
#include "fileio.h"
#endif
#ifndef __ENCODE_BUFFER_H__
#include "encode_buffer.h"
#endif




/* ///////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(IJLERR,EncodeHuffman8x8,(
  Ipp16s*        pSrc,
  Ipp16s*        pLastDC,
  HUFFMAN_TABLE* pDcTable,
  HUFFMAN_TABLE* pAcTable,
  STATE*         pState,
  int            bIsLast));

OWNAPI(IJLERR,Make_optimal_huff_table,(
  int    FREQ[256],
  Ipp8u* BITS,
  Ipp8u* VALS));

OWNAPI(IJLERR,Gather_Prog_DC_SA,(
  Ipp16s* pSrc,
  Ipp16s* pLastDC,
  int     Al,
  int     statistics[256]));

OWNAPI(IJLERR,Gather_Prog_AC_first,(
  Ipp16s* pSrc,
  int     Ss,
  int     Se,
  int     Al,
  int     FREQ[256],
  STATE*  pState,
  int     bIsLast));

OWNAPI(IJLERR,Gather_Prog_AC_refine,(
  Ipp16s* pSrc,
  int     Ss,
  int     Se,
  int     Al,
  int     FREQ[256],
  STATE*  pState,
  int     bIsLast));

OWNAPI(IJLERR,Encode_Prog_DC_SA,(
  Ipp16s*        pSrc,
  Ipp16s*        pLastDC,
  int            Ah,
  int            Al,
  HUFFMAN_TABLE* pDcTable,
  STATE*         pState,
  int            bIsLast));

OWNAPI(IJLERR,Encode_Prog_AC_first,(
  Ipp16s*        pSrc,
  int            Ss,
  int            Se,
  int            Al,
  HUFFMAN_TABLE* pAcTable,
  STATE*         pState,
  int            bIsLast));

OWNAPI(IJLERR,Encode_Prog_AC_refine,(
  Ipp16s*        pSrc,
  int            Ss,
  int            Se,
  int            Al,
  HUFFMAN_TABLE* pAcTable,
  STATE*         pState,
  int            bIsLast));


#endif /* __HUFF_ENCODE_H__ */

