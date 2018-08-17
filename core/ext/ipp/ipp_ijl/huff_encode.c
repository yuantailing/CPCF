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
#ifndef __HUFF_ENCODE_H__
#include "huff_encode.h"
#endif



OWNFUN(IJLERR) Make_optimal_huff_table(
  int    freq[256],
  Ipp8u* bits,
  Ipp8u* vals)
{
  IppStatus status;
  status = IPPCALL(ippiEncodeHuffmanRawTableInit_JPEG_8u)(freq,bits,vals);

  if(ippStsNoErr != status)
  {
    return IJL_INTERNAL_ERROR;
  }

  return IJL_OK;
} /* Make_optimal_huff_table() */


/*F*
////////////////////////////////////////////////////////////////////////////
// Name
//   EncodeHuffman8x8
//
// Purpose
//   Encode frequency components in block into the output JPEG data stream
//   using the huffman tables dc_table and ac_table.
//
// Context
//
// Returns
//   valid error code, or 0 for OK
//
// Parameters
//   pSrc        pointer to the 8x8 block of DCT coefficients
//   pLastDC     DC value from last block of this channel
//               used to generate current DC value via DPCM
//   pDcTable    dc Huffman table used on this 8x8 block
//   pAcTable    ac Huffman table used on this 8x8 block
//   pState      Pointer to current IJL Encoder state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EncodeHuffman8x8(
  Ipp16s*        pSrc,
  Ipp16s*        pLastDC,
  HUFFMAN_TABLE* pDcTable,
  HUFFMAN_TABLE* pAcTable,
  STATE*         pState,
  int            bIsLast)
{
  int pos = 0;
  IppStatus status;
  IJLERR jerr = IJL_OK;

  /* protect against buffer overrun */
  if(pState->entropy_bytes_left <= 128)
  {
    if(NULL == pState->file)
    {
      jerr = IJL_BUFFER_TOO_SMALL;
      goto Exit;
    }

    jerr = Flush_Buffer_To_File(pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

  status = IPPCALL(ippiEncodeHuffman8x8_JPEG_16s1u_C1)(
    pSrc,
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    pLastDC,
    pDcTable->u.pEncHuffTbl,
    pAcTable->u.pEncHuffTbl,
    pState->u.pEncHuffState,
    bIsLast);

  if(ippStsNoErr != status)
  {
    jerr = IJL_INTERNAL_ERROR;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

Exit:

  return jerr;
} /* EncodeHuffman8x8() */


OWNFUN(IJLERR) Gather_Prog_DC_SA(
  Ipp16s* pSrc,
  Ipp16s* pLastDC,
  int     Al,
  int     statistics[256])
{
  IppStatus status;

  status = IPPCALL(ippiGetHuffmanStatistics8x8_DCFirst_JPEG_16s_C1)(
    pSrc,
    statistics,
    pLastDC,
    Al);

  if(ippStsNoErr != status)
  {
    return IJL_INTERNAL_ERROR;
  }

  return IJL_OK;
} /* Gather_Prog_DC_SA() */


OWNFUN(IJLERR) Encode_Prog_DC_SA(
  Ipp16s*        pSrc,
  Ipp16s*        pLastDC,
  int            Ah,
  int            Al,
  HUFFMAN_TABLE* pDcTable,
  STATE*         pState,
  int            bIsLast)
{
  int pos = 0;
  IppStatus status;
  IJLERR jerr = IJL_OK;

  /* protect against buffer overrun */
  if(pState->entropy_bytes_left <= 128)
  {
    if(NULL == pState->file)
    {
      jerr = IJL_BUFFER_TOO_SMALL;
      goto Exit;
    }

    jerr = Flush_Buffer_To_File(pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

  if(Ah == 0)
  {
    status = IPPCALL(ippiEncodeHuffman8x8_DCFirst_JPEG_16s1u_C1)(
      pSrc,
      pState->cur_entropy_ptr,
      pState->entropy_bytes_left,
      &pos,
      pLastDC,
      Al,
      pDcTable->u.pEncHuffTbl,
      pState->u.pEncHuffState,
      bIsLast);
  }
  else
  {
    status = IPPCALL(ippiEncodeHuffman8x8_DCRefine_JPEG_16s1u_C1)(
      pSrc,
      pState->cur_entropy_ptr,
      pState->entropy_bytes_left,
      &pos,
      Al,
      pState->u.pEncHuffState,
      bIsLast);
  }

  if(ippStsNoErr != status)
  {
    jerr = IJL_INTERNAL_ERROR;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

Exit:

  return jerr;
} /* Encode_Prog_DC_SA() */


OWNFUN(IJLERR) Gather_Prog_AC_first(
  Ipp16s* pSrc,
  int     Ss,
  int     Se,
  int     Al,
  int     FREQ[256],
  STATE*  pState,
  int     bIsLast)
{
  IppStatus status;
  status = IPPCALL(ippiGetHuffmanStatistics8x8_ACFirst_JPEG_16s_C1)(
    pSrc,
    FREQ,
    Ss,
    Se,
    Al,
    pState->u.pEncHuffState,
    bIsLast);

  if(ippStsNoErr != status)
  {
    return IJL_INTERNAL_ERROR;
  }

  return IJL_OK;
} /* Gather_Prog_AC_first() */


OWNFUN(IJLERR) Gather_Prog_AC_refine(
  Ipp16s* pSrc,
  int     Ss,
  int     Se,
  int     Al,
  int     FREQ[256],
  STATE*  pState,
  int     bIsLast)
{
  IppStatus status;
  status = IPPCALL(ippiGetHuffmanStatistics8x8_ACRefine_JPEG_16s_C1)(
    pSrc,
    FREQ,
    Ss,
    Se,
    Al,
    pState->u.pEncHuffState,
    bIsLast);

  if(ippStsNoErr != status)
  {
    return IJL_INTERNAL_ERROR;
  }

  return IJL_OK;
} /* Gather_Prog_AC_refine() */


OWNFUN(IJLERR) Encode_Prog_AC_first(
  Ipp16s*        pSrc,
  int            Ss,
  int            Se,
  int            Al,
  HUFFMAN_TABLE* pAcTable,
  STATE*         pState,
  int            bIsLast)
{
  int pos = 0;
  IppStatus status;
  IJLERR jerr = IJL_OK;

  /* protect against buffer overrun */
  if(pState->entropy_bytes_left <= 128)
  {
    if(NULL == pState->file)
    {
      jerr = IJL_BUFFER_TOO_SMALL;
      goto Exit;
    }

    jerr = Flush_Buffer_To_File(pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

  status = IPPCALL(ippiEncodeHuffman8x8_ACFirst_JPEG_16s1u_C1)(
    pSrc,
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    Ss,
    Se,
    Al,
    pAcTable->u.pEncHuffTbl,
    pState->u.pEncHuffState,
    bIsLast);

  if(ippStsNoErr != status)
  {
    jerr = IJL_INTERNAL_ERROR;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

Exit:

  return IJL_OK;
} /* Encode_Prog_AC_first() */


OWNFUN(IJLERR) Encode_Prog_AC_refine(
  Ipp16s*        pSrc,
  int            Ss,
  int            Se,
  int            Al,
  HUFFMAN_TABLE* pAcTable,
  STATE*         pState,
  int            bIsLast)
{
  int pos = 0;
  IppStatus status;
  IJLERR jerr = IJL_OK;

  /* protect against buffer overrun */
  if(pState->entropy_bytes_left <= 128)
  {
    if(NULL == pState->file)
    {
      jerr = IJL_BUFFER_TOO_SMALL;
      goto Exit;
    }

    jerr = Flush_Buffer_To_File(pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

  status = IPPCALL(ippiEncodeHuffman8x8_ACRefine_JPEG_16s1u_C1)(
    pSrc,
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    Ss,
    Se,
    Al,
    pAcTable->u.pEncHuffTbl,
    pState->u.pEncHuffState,
    bIsLast);

  if(ippStsNoErr != status)
  {
    jerr = IJL_INTERNAL_ERROR;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

Exit:

  return IJL_OK;
} /* Encode_Prog_AC_refine() */


