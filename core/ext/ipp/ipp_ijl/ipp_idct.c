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
#ifndef __IPP_IDCT_H__
#include "ipp_idct.h"
#endif



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    FillDecoderQuantTable
//
//  Purpose
//
//  Context
//
//  Returns
//    IJL_OK or error code
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) FillDecoderQuantTable(
  Ipp8u*  pRawQuantTbl,
  Ipp16u* pDecQuantTbl)
{
  IppStatus status;

  status = IPPCALL(ippiQuantInvTableInit_JPEG_8u16u)(pRawQuantTbl,pDecQuantTbl);
  if(ippStsNoErr != status)
    return IJL_BAD_QUANT_TABLE;

  return IJL_OK;
} /* FillDecoderQuantTable() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    iQnt8x8
//
//  Purpose
//
//  Context
//
//  Parameters
//    short* coef_block
//    short* quant_table
//
//  Returns
//    IJL_OK or error code
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) iQnt8x8(
  Ipp16s* pSrcDst,
  Ipp16u* pDecQuantTbl)
{
  IppStatus status;

  status = IPPCALL(ippiQuantInv8x8_JPEG_16s_C1I)(pSrcDst,pDecQuantTbl);
  if(ippStsNoErr != status)
    return IJL_BAD_QUANT_TABLE;

  return IJL_OK;
} /* iQnt8x8() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    iDCT8x8
//
//  Purpose
//    Performs an inverse DCT on an 8x8 block.
//
//  Context
//
//  Returns
//    IJL_OK or error code
//
//  Parameters
//    pSrcDst  Input:  A set of 1 DC and 63 AC coefficients.
//             Output: An 8x8 raster of image values.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) iDCT8x8(
  Ipp16s* pSrcDst)
{
  IppStatus status;

  status = IPPCALL(ippiDCT8x8Inv_16s_C1I)(pSrcDst);
  if(ippStsNoErr != status)
    return IJL_INTERNAL_ERROR;

  return IJL_OK;
} /* iDCT8x8() */


OWNFUN(IJLERR) iDCT4x4(
  Ipp16s* pSrcDst)
{
  int       i;
  Ipp16s    bf[4*4];
  IppStatus status;

  status = IPPCALL(ippiDCT8x8To4x4Inv_16s_C1)(pSrcDst,bf);
  if(ippStsNoErr != status)
    return IJL_INTERNAL_ERROR;

  for(i = 0; i < 4; i++)
  {
    pSrcDst[i*8 + 0] = bf[i*4 + 0];
    pSrcDst[i*8 + 1] = bf[i*4 + 1];
    pSrcDst[i*8 + 2] = bf[i*4 + 2];
    pSrcDst[i*8 + 3] = bf[i*4 + 3];
  }

  return IJL_OK;
} /* iDCT4x4() */


OWNFUN(IJLERR) iDCT2x2(
  Ipp16s* pSrcDst)
{
  int       i;
  Ipp16s    bf[2*2];
  IppStatus status;

  status = IPPCALL(ippiDCT8x8To2x2Inv_16s_C1)(pSrcDst,bf);
  if(ippStsNoErr != status)
    return IJL_INTERNAL_ERROR;

  for(i = 0; i < 2; i++)
  {
    pSrcDst[i*8 + 0] = bf[i*2 + 0];
    pSrcDst[i*8 + 1] = bf[i*2 + 1];
  }

  return IJL_OK;
} /* iDCT2x2() */


OWNFUN(IJLERR) iDCT1x1(
  Ipp16s* pSrcDst)
{
  pSrcDst[0] = (Ipp16s)(pSrcDst[0] >> 3);
  return IJL_OK;
} /* iDCT1x1() */

