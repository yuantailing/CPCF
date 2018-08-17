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
#ifndef __IPP_FDCT_H__
#include "ipp_fdct.h"
#endif



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    FillEncoderQuantTable
//
//  Purpose:
//
//  Context:
//
//  Returns:
//    IJL_OK or error code
//
//  Parameters:
//    pRawQuantTbl
//    pEncQuantTbl
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) FillEncoderQuantTable(
  Ipp8u*  pRawQuantTbl,
  Ipp16u* pEncQuantTbl)
{
  IppStatus status;

  status = IPPCALL(ippiQuantFwdTableInit_JPEG_8u16u)(pRawQuantTbl,pEncQuantTbl);
  if(ippStsNoErr != status)
  {
    return IJL_BAD_QUANT_TABLE;
  }

  return IJL_OK;
} /* FillEncoderQuantTable() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    fQnt8x8
//
//  Purpose
//    Perform in-place quantization on a single block of frequency components.
//
//  Context
//
//  Returns
//    valid error code, or 0 if OK
//
//  Parameters
//    pSrcDst       Pointer to a 8x8 array of frequency-domain data
//    pEncQuantTbl  Pointer to a quantization table.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) fQnt8x8(
  Ipp16s* pSrcDst,
  Ipp16u* pEncQuantTbl)
{
  IppStatus status;

  status = IPPCALL(ippiQuantFwd8x8_JPEG_16s_C1I)(pSrcDst,pEncQuantTbl);
  if(ippStsNoErr != status)
  {
    return IJL_BAD_QUANT_TABLE;
  }

  return IJL_OK;
} /* fQnt8x8() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    fDCT8x8
//
//  Purpose
//    Performs a forward DCT on an 8x8 block.
//
//  Context
//
//  Returns
//    valid error code, or 0 if OK
//
//  Parameters
//    pSrcDst  Pointer to a 8x8 block of DCT coefficient
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) fDCT8x8(
  Ipp16s* pSrcDst)
{
  IppStatus status;

  status = IPPCALL(ippiDCT8x8Fwd_16s_C1I)(pSrcDst);
  if(ippStsNoErr != status)
  {
    return IJL_INTERNAL_ERROR;
  }

  return IJL_OK;
} /* fDCT8x8() */


