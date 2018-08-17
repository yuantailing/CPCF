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
#ifndef __HUFF_DECODE_H__
#include "huff_decode.h"
#endif



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DecodeHuffman8x8
//
//  Purpose
//    Decode the Huffman-encoded entropy bitstream for a single 8x8 block.
//    This function uses the macros defined above, and works for input
//    defined either from a buffer or from a file. This routine is one of the
//    3 most time-consuming portions of the JPEG decoder, occupying over 30%
//    of the total time required to decode an image.
//
//  Context
//
//  Returns
//    valid error code, or 0 for OK
//
//  Parameters
//    pState      The pointer to IJL state variables
//                (technically a member of JPEG_PROPERTIES)
//    pDcTable    The pointer to DC huffman tables for this 8x8 block
//    pAcTable    The pointer to AC huffman tables for this 8x8 block
//    pLastDC     The pointer to DC value of the last 8x8 block in the
//                same channel as this 8x8 block. Used for the
//                DPCM calculation of the DC coefficient.
//    pDst        The pointer to output 8x8 block of DCT coefficient
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DecodeHuffman8x8(
  STATE*         pState,
  HUFFMAN_TABLE* pDcTable,
  HUFFMAN_TABLE* pAcTable,
  Ipp16s*        pLastDC,
  Ipp16s*        pDst)
{
  int pos = 0;
  IJLERR jerr;
  IppStatus status;

  if(pState->entropy_bytes_left < 128)
  {
    jerr = Buffer_Read_Bytes(pState);
    if(IJL_OK != jerr)
      return jerr;
  }

  status = IPPCALL(ippiDecodeHuffman8x8_JPEG_1u16s_C1)(
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    pDst,
    pLastDC,
    &pState->unread_marker,
    pDcTable->u.pDecHuffTbl,
    pAcTable->u.pDecHuffTbl,
    pState->u.pDecHuffState);

  if(ippStsNoErr != status)
  {
    jerr = IJL_OK;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

  return IJL_OK;
} /* DecodeHuffman8x8() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Decode_Block_Prog_DC
//
//  Purpose:
//
//  Context:
//
//  Returns:
//    Valid error code, or 0 for OK.
//
//  Parameters:
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Decode_Block_Prog_DC(
  STATE*         pState,
  HUFFMAN_TABLE* pDcTable,
  Ipp16s*        pDst,
  Ipp16s*        pLastDC,
  int            Al)
{
  int pos = 0;
  IJLERR jerr;
  IppStatus status;

  if(pState->entropy_bytes_left < 128)
  {
    jerr = Buffer_Read_Bytes(pState);
    if(IJL_OK != jerr)
      return jerr;
  }

  status = IPPCALL(ippiDecodeHuffman8x8_DCFirst_JPEG_1u16s_C1)(
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    pDst,
    pLastDC,
    &pState->unread_marker,
    Al,
    pDcTable->u.pDecHuffTbl,
    pState->u.pDecHuffState);

  if(ippStsNoErr != status)
  {
    jerr = IJL_OK;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

  return IJL_OK;
} /* Decode_Block_Prog_DC() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Decode_Block_Prog_DC_SA
//
//  Purpose:
//
//  Context:
//
//  Returns:
//    Valid error code, or 0 for OK.
//
//  Parameters:
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Decode_Block_Prog_DC_SA(
  STATE*  pState,
  Ipp16s* pDst,
  int     Al)
{
  int pos = 0;
  IJLERR jerr;
  IppStatus status;

  if(pState->entropy_bytes_left < 128)
  {
    jerr = Buffer_Read_Bytes(pState);
    if(IJL_OK != jerr)
      return jerr;
  }

  status = IPPCALL(ippiDecodeHuffman8x8_DCRefine_JPEG_1u16s_C1)(
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    pDst,
    &pState->unread_marker,
    Al,
    pState->u.pDecHuffState);

  if(ippStsNoErr != status)
  {
    jerr = IJL_OK;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

  return IJL_OK;
} /* Decode_Block_Prog_DC_SA() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Decode_Block_Prog_AC
//
//  Purpose:
//
//  Context:
//
//  Returns:
//    Valid error code, or 0 for OK.
//
//  Parameters:
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Decode_Block_Prog_AC(
  STATE*         pState,
  Ipp16s*        pDst,
  HUFFMAN_TABLE* pAcTable,
  int            Ss,
  int            Se,
  int            Al)
{
  int pos = 0;
  IJLERR jerr;
  IppStatus status;

  if(pState->entropy_bytes_left < 128)
  {
    jerr = Buffer_Read_Bytes(pState);
    if(IJL_OK != jerr)
      return jerr;
  }

  status = IPPCALL(ippiDecodeHuffman8x8_ACFirst_JPEG_1u16s_C1)(
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    pDst,
    &pState->unread_marker,
    Ss,
    Se,
    Al,
    pAcTable->u.pDecHuffTbl,
    pState->u.pDecHuffState);

  if(ippStsNoErr != status)
  {
    jerr = IJL_OK;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

  return IJL_OK;
} /* Decode_Block_Prog_AC() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Decode_Block_Prog_AC_SA
//
//  Purpose:
//
//  Context:
//
//  Returns:
//    Valid error code, or 0 for OK.
//
//  Parameters:
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Decode_Block_Prog_AC_SA(
  STATE*         pState,
  Ipp16s*        pDst,
  HUFFMAN_TABLE* pAcTable,
  int            Ss,
  int            Se,
  int            Al)
{
  int pos = 0;
  IJLERR jerr;
  IppStatus status;

  if(pState->entropy_bytes_left < 128)
  {
    jerr = Buffer_Read_Bytes(pState);
    if(IJL_OK != jerr)
      return jerr;
  }

  status = IPPCALL(ippiDecodeHuffman8x8_ACRefine_JPEG_1u16s_C1)(
    pState->cur_entropy_ptr,
    pState->entropy_bytes_left,
    &pos,
    pDst,
    &pState->unread_marker,
    Ss,
    Se,
    Al,
    pAcTable->u.pDecHuffTbl,
    pState->u.pDecHuffState);

  if(ippStsNoErr != status)
  {
    jerr = IJL_OK;
  }

  pState->cur_entropy_ptr         += pos;
  pState->entropy_bytes_left      -= pos;
  pState->entropy_bytes_processed += pos;

  return IJL_OK;
}  /* Decode_Block_Prog_AC_SA */

