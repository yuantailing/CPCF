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
#ifndef CC_SS_ENCODER_H__
#include "cc_ss_encoder.h"
#endif



/* ///////////////////////////////////////////////////////////////////////////
// Macros/Constants
*/

/*
// The following routines implement the color conversion equations
// from CCIR Recommendation 601.
//
// In floating point notation, RGB->YCbCr:
//    Y  =  ( 0.29900*R) + (0.58700*G) + (0.11400*B)
//    Cb = ((-0.16874*R) - (0.33126*G) + (0.50000*B)) + 128.00000
//    Cr = (( 0.50000*R) - (0.41869*G) - (0.08131*B)) + 128.00000
//
// In fixed point notation (using 8 bits), RGB->YCbCr:
//    Y  = (((77*R) + (150*G) + (29*B)) / 256) - 128
//       =  ((77*R) + (150*G) + (29*B) - 32768) / 256
//       =  ((77*R) + (150*G) + (29*B) - 32768 + 128) >> 8
//       =  ((77*R) + (150*G) + (29*B) - 32640) >> 8
//
//    Cb = (((-43*R) - (85*G) + (128*B) + 32768) / 256) - 128
//       =  ((-43*R) - (85*G) + (128*B) + 32768 - 32768) / 256
//       =  ((-43*R) - (85*G) + (128*B) + 32768 - 32768 + 128) >> 8
//       =  ((-43*R) - (85*G) + (128*B) + 128) >> 8
//       =  ((-43*R) - (85*G) + (B<<7)  + 128) >> 8
//
//    Cr = (((128*R) - (107*G) - (21*B) + 32768) / 256) - 128
//       =  ((128*R) - (107*G) - (21*B) + 32768 - 32768) / 256
//       =  ((128*R) - (107*G) - (21*B) + 32768 - 32768 + 128) >> 8
//       =  ((128*R) - (107*G) - (21*B) + 128) >> 8
//       =  ((R<<7)  - (107*G) - (21*B) + 128) >> 8
//
//    Note that a rounding technique for the right-shift operation is
//    incorporated directly into these fixed point equations
//    (i.e., a right-shift of N is preceded by adding 2^(N-1)).
//
//    Also note that a "level shift" of -128 is needed to set up for
//    the forward DCT.  This level shift has been incorporated directly
//    into these fixed point equations.
//
*/

#define CC_RGB_Y(R,G,B)       (((77*(R)) + (150*(G)) + (29*(B)) - 32640) >> 8)

#define CC_RGB_CB(R,G,B)      (((-43*(R)) - (85*(G)) + ((B)<<7)  + 128) >> 8)
#define CC_RGB_CR(R,G,B)      ((((R)<<7)  - (107*(G)) - (21*(B)) + 128) >> 8)

#define CC_RGB_CB_444(R,G,B)  (((-11059*(R)) - (21709*(G)) +  ((B)<<15)) >> 16)
#define CC_RGB_CR_444(R,G,B)  (((   (R)<<15) - (27439*(G)) - (5329*(B))) >> 16)

#define CC_RGB_CB_422(R,G,B)  (((-11059*(R)) - (21709*(G)) +  ((B)<<15)) >> 17)
#define CC_RGB_CR_422(R,G,B)  (((   (R)<<15) - (27439*(G)) - (5329*(B))) >> 17)

#define CC_RGB_CB_411(R,G,B)  (((-11059*(R)) - (21709*(G)) +  ((B)<<15)) >> 18)
#define CC_RGB_CR_411(R,G,B)  (((   (R)<<15) - (27439*(G)) - (5329*(B))) >> 18)


LOCFUN(IppStatus) ippiCopyExpand_8u_C1R(
  const Ipp8u*   pSrc,
        int      srcStep,
        IppiSize srcRoi,
        Ipp8u*   pDst,
        int      dstStep,
        IppiSize dstRoi)
{
  int i;
  int j;
  const Ipp8u* pCurrSrc;
  Ipp8u* pCurrDst;

  if(NULL == pSrc || NULL == pDst)
    return ippStsNullPtrErr;

  if(srcStep < 0 || dstStep < 0)
    return ippStsStepErr;

  if(srcRoi.width < 0 || srcRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width < 0 || dstRoi.height < 0)
    return ippStsSizeErr;

  IPPCALL(ippiCopy_8u_C1R)(pSrc,srcStep,pDst,dstStep,srcRoi);

  if(dstRoi.width > srcRoi.width)
  {
    /* expand source image most-right column */
    for(j = 0; j < srcRoi.height; j++)
    {
      pCurrSrc = pSrc + j*srcStep;
      pCurrDst = pDst + j*dstStep;
      for(i = srcRoi.width; i < dstRoi.width; i++)
      {
        pCurrDst[i] = pCurrSrc[srcRoi.width - 1];
      }
    }
  }

  if(dstRoi.height > srcRoi.height)
  {
    pCurrSrc = pDst + (srcRoi.height - 1)*dstStep;
    for(j = srcRoi.height; j < dstRoi.height; j++)
    {
      pCurrDst = pDst + j*dstStep;
      IPPCALL(ippsCopy_8u)(pCurrSrc,pCurrDst,dstRoi.width);
    }
  }

  return ippStsNoErr;
} /* ippiCopyExpand_8u_C1R() */


LOCFUN(IppStatus) ippiExpand_8u_C1IR(
  Ipp8u*   pSrcDst,
  int      srcDstStep,
  IppiSize srcRoi,
  IppiSize dstRoi)
{
  int i;
  int j;
  Ipp8u* pCurrSrc;
  Ipp8u* pCurrDst;

  if(NULL == pSrcDst)
    return ippStsNullPtrErr;

  if(srcDstStep < 0)
    return ippStsStepErr;

  if(srcRoi.width < 0 || srcRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width < 0 || dstRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width > srcRoi.width)
  {
    /* expand source image most-right column */
    for(j = 0; j < srcRoi.height; j++)
    {
      pCurrSrc = pSrcDst + j*srcDstStep;
      pCurrDst = pSrcDst + j*srcDstStep;
      for(i = srcRoi.width; i < dstRoi.width; i++)
      {
        pCurrDst[i] = pCurrSrc[srcRoi.width - 1];
      }
    }
  }

  if(dstRoi.height > srcRoi.height)
  {
    pCurrSrc = pSrcDst + (srcRoi.height - 1)*srcDstStep;
    for(j = srcRoi.height; j < dstRoi.height; j++)
    {
      pCurrDst = pSrcDst + j*srcDstStep;
      IPPCALL(ippsCopy_8u)(pCurrSrc,pCurrDst,dstRoi.width);
    }
  }

  return ippStsNoErr;
} /* ippiExpand_8u_C1IR() */


LOCFUN(IppStatus) ippiCopyExpand_8u_C3R(
  const Ipp8u*   pSrc,
        int      srcStep,
        IppiSize srcRoi,
        Ipp8u*   pDst,
        int      dstStep,
        IppiSize dstRoi)
{
  int i;
  int j;
  const Ipp8u* pCurrSrc;
  Ipp8u* pCurrDst;

  if(NULL == pSrc || NULL == pDst)
    return ippStsNullPtrErr;

  if(srcStep < 0 || dstStep < 0)
    return ippStsStepErr;

  if(srcRoi.width < 0 || srcRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width < 0 || dstRoi.height < 0)
    return ippStsSizeErr;

  IPPCALL(ippiCopy_8u_C3R)(pSrc,srcStep,pDst,dstStep,srcRoi);

  if(dstRoi.width > srcRoi.width)
  {
    /* expand source image most-right column */
    for(j = 0; j < srcRoi.height; j++)
    {
      pCurrSrc = pSrc + j*srcStep;
      pCurrDst = pDst + j*dstStep;
      for(i = srcRoi.width; i < dstRoi.width; i++)
      {
        pCurrDst[i*3+0] = pCurrSrc[srcRoi.width*3 - 3 + 0];
        pCurrDst[i*3+1] = pCurrSrc[srcRoi.width*3 - 3 + 1];
        pCurrDst[i*3+2] = pCurrSrc[srcRoi.width*3 - 3 + 2];
      }
    }
  }

  if(dstRoi.height > srcRoi.height)
  {
    pCurrSrc = pDst + (srcRoi.height - 1)*dstStep;
    for(j = srcRoi.height; j < dstRoi.height; j++)
    {
      pCurrDst = pDst + j*dstStep;
      IPPCALL(ippsCopy_8u)(pCurrSrc,pCurrDst,dstRoi.width*3);
    }
  }

  return ippStsNoErr;
} /* ippiCopyExpand_8u_C3R() */


LOCFUN(IppStatus) ippiExpand_8u_C3IR(
  Ipp8u*   pSrcDst,
  int      srcDstStep,
  IppiSize srcRoi,
  IppiSize dstRoi)
{
  int i;
  int j;
  Ipp8u* pCurrSrc;
  Ipp8u* pCurrDst;

  if(NULL == pSrcDst)
    return ippStsNullPtrErr;

  if(srcDstStep < 0)
    return ippStsStepErr;

  if(srcRoi.width < 0 || srcRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width < 0 || dstRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width > srcRoi.width)
  {
    /* expand source image most-right column */
    for(j = 0; j < srcRoi.height; j++)
    {
      pCurrSrc = pSrcDst + j*srcDstStep;
      pCurrDst = pSrcDst + j*srcDstStep;
      for(i = srcRoi.width; i < dstRoi.width; i++)
      {
        pCurrDst[i*3+0] = pCurrSrc[srcRoi.width*3 - 3 + 0];
        pCurrDst[i*3+1] = pCurrSrc[srcRoi.width*3 - 3 + 1];
        pCurrDst[i*3+2] = pCurrSrc[srcRoi.width*3 - 3 + 2];
      }
    }
  }

  if(dstRoi.height > srcRoi.height)
  {
    pCurrSrc = pSrcDst + (srcRoi.height - 1)*srcDstStep;
    for(j = srcRoi.height; j < dstRoi.height; j++)
    {
      pCurrDst = pSrcDst + j*srcDstStep;
      IPPCALL(ippsCopy_8u)(pCurrSrc,pCurrDst,dstRoi.width*3);
    }
  }

  return ippStsNoErr;
} /* ippiExpand_8u_C3IR() */


LOCFUN(IppStatus) ippiCopyExpand_8u_C4R(
  const Ipp8u*   pSrc,
        int      srcStep,
        IppiSize srcRoi,
        Ipp8u*   pDst,
        int      dstStep,
        IppiSize dstRoi)
{
  int i;
  int j;
  const Ipp8u* pCurrSrc;
  Ipp8u* pCurrDst;

  if(NULL == pSrc || NULL == pDst)
    return ippStsNullPtrErr;

  if(srcStep < 0 || dstStep < 0)
    return ippStsStepErr;

  if(srcRoi.width < 0 || srcRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width < 0 || dstRoi.height < 0)
    return ippStsSizeErr;

  IPPCALL(ippiCopy_8u_C4R)(pSrc,srcStep,pDst,dstStep,srcRoi);

  if(dstRoi.width > srcRoi.width)
  {
    /* expand source image most-right column */
    for(j = 0; j < srcRoi.height; j++)
    {
      pCurrSrc = pSrc + j*srcStep;
      pCurrDst = pDst + j*dstStep;
      for(i = srcRoi.width; i < dstRoi.width; i++)
      {
        pCurrDst[i*4+0] = pCurrSrc[srcRoi.width*4 - 4 + 0];
        pCurrDst[i*4+1] = pCurrSrc[srcRoi.width*4 - 4 + 1];
        pCurrDst[i*4+2] = pCurrSrc[srcRoi.width*4 - 4 + 2];
        pCurrDst[i*4+3] = pCurrSrc[srcRoi.width*4 - 4 + 3];
      }
    }
  }

  if(dstRoi.height > srcRoi.height)
  {
    pCurrSrc = pDst + (srcRoi.height - 1)*dstStep;
    for(j = srcRoi.height; j < dstRoi.height; j++)
    {
      pCurrDst = pDst + j*dstStep;
      IPPCALL(ippsCopy_8u)(pCurrSrc,pCurrDst,dstRoi.width*4);
    }
  }

  return ippStsNoErr;
} /* ippiCopyExpand_8u_C4R() */


LOCFUN(IppStatus) ippiExpand_8u_C4IR(
  Ipp8u*   pSrcDst,
  int      srcDstStep,
  IppiSize srcRoi,
  IppiSize dstRoi)
{
  int i;
  int j;
  Ipp8u* pCurrSrc;
  Ipp8u* pCurrDst;

  if(NULL == pSrcDst)
    return ippStsNullPtrErr;

  if(srcDstStep < 0)
    return ippStsStepErr;

  if(srcRoi.width < 0 || srcRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width < 0 || dstRoi.height < 0)
    return ippStsSizeErr;

  if(dstRoi.width > srcRoi.width)
  {
    /* expand source image most-right column */
    for(j = 0; j < srcRoi.height; j++)
    {
      pCurrSrc = pSrcDst + j*srcDstStep;
      pCurrDst = pSrcDst + j*srcDstStep;
      for(i = srcRoi.width; i < dstRoi.width; i++)
      {
        pCurrDst[i*4+0] = pCurrSrc[srcRoi.width*4 - 4 + 0];
        pCurrDst[i*4+1] = pCurrSrc[srcRoi.width*4 - 4 + 1];
        pCurrDst[i*4+2] = pCurrSrc[srcRoi.width*4 - 4 + 2];
        pCurrDst[i*4+3] = pCurrSrc[srcRoi.width*4 - 4 + 3];
      }
    }
  }

  if(dstRoi.height > srcRoi.height)
  {
    pCurrSrc = pSrcDst + (srcRoi.height - 1)*srcDstStep;
    for(j = srcRoi.height; j < dstRoi.height; j++)
    {
      pCurrDst = pSrcDst + j*srcDstStep;
      IPPCALL(ippsCopy_8u)(pCurrSrc,pCurrDst,dstRoi.width*4);
    }
  }

  return ippStsNoErr;
} /* ippiExpand_8u_C4IR() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    RGB_to_YCbCr_111_MCU
//
//  Purpose
//    Color convert a MCU of 3-channel RGB DIB input data into
//    1:1:1 YCbCr format.
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) RGB_to_YCbCr_111_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 8;
  int     y          = 8;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[8*8*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,8*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -=(pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,8*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,8*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*1];
    pDst[2] = &outptr[DCTSIZE2*2];

    IPPCALL(ippiRGBToYCbCr444LS_MCU_8u16s_C3P3R)(pBuf,8*3,pDst);
  }

  return;
} /* RGB_to_YCbCr_111_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    BGR_to_YCbCr_111_MCU
//
//  Purpose
//    Color convert a MCU of 3-channel BGR DIB input data into
//    1:1:1 YCbCr format.
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) BGR_to_YCbCr_111_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 8;
  int     y          = 8;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[8*8*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,8*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,8*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,8*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*1];
    pDst[2] = &outptr[DCTSIZE2*2];

    IPPCALL(ippiBGRToYCbCr444LS_MCU_8u16s_C3P3R)(pBuf,8*3,pDst);
  }

  return;
} /* BGR_to_YCbCr_111_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    RGB_to_YCbCr_411_MCU
//
//  Purpose
//    Color convert a MCU of 3-channel RGB DIB input data into
//    4:1:1 YCbCr format.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) RGB_to_YCbCr_411_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 16;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*16*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,16*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,16*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,16*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*4];
    pDst[2] = &outptr[DCTSIZE2*5];

    IPPCALL(ippiRGBToYCbCr411LS_MCU_8u16s_C3P3R)(pBuf,16*3,pDst);
  }

  return;
} /* RGB_to_YCbCr_411_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    BGR_to_YCbCr_411_MCU
//
//  Purpose
//    Color convert a MCU of 3-channel BGR DIB input data into
//    4:1:1 YCbCr format.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) BGR_to_YCbCr_411_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 16;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*16*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,16*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,16*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,16*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*4];
    pDst[2] = &outptr[DCTSIZE2*5];

    IPPCALL(ippiBGRToYCbCr411LS_MCU_8u16s_C3P3R)(pBuf,16*3,pDst);
  }

  return;
} /* BGR_to_YCbCr_411_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    RGB_to_YCbCr_422_MCU
//
//  Purpose
//    Color convert a MCU of 3-channel RGB DIB input data into
//    4:2:2 YCbCr format.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) RGB_to_YCbCr_422_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 8;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*8*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,16*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,16*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,16*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*2];
    pDst[2] = &outptr[DCTSIZE2*3];

    IPPCALL(ippiRGBToYCbCr422LS_MCU_8u16s_C3P3R)(pBuf,16*3,pDst);
  }

  return;
} /* RGB_to_YCbCr_422_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    BGR_to_YCbCr_422_MCU
//
//  Purpose
//    Color convert a MCU of 3-channel BGR DIB input data into
//    4:2:2 YCbCr format.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) BGR_to_YCbCr_422_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 8;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*8*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,16*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,16*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,16*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*2];
    pDst[2] = &outptr[DCTSIZE2*3];

    IPPCALL(ippiBGRToYCbCr422LS_MCU_8u16s_C3P3R)(pBuf,16*3,pDst);
  }

  return;
} /* BGR_to_YCbCr_422_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    RGBA_FPX_to_YCbCrA_FPX_1111_MCU
//
//  Purpose
//    Color convert a MCU of 4-channel RGBA_FPX DIB input data into
//    1:1:1:1 YCbCrA_FPX format.
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) RGBA_FPX_to_YCbCrA_FPX_1111_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 8;
  int     y          = 8;
  int     bytes      = 4;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (short*)jprops->MCUBuf;
  Ipp8u   tmp[8*8*4+CPU_CACHE_LINE];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[4];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C4R(inptr,lineoffset,realSize,pBuf,8*4,wishSize);
    }
    else
    {
      IppiSize realSizeC1;

      realSizeC1.width  = pixwidth*4;
      realSizeC1.height = pixheight;

      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1R)(inptr,lineoffset,pBuf,8*4,realSizeC1,ippAxsHorizontal);
      ippiExpand_8u_C4IR(pBuf,8*4,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*1];
    pDst[2] = &outptr[DCTSIZE2*2];
    pDst[3] = &outptr[DCTSIZE2*3];

    IPPCALL(ippiCMYKToYCCK444LS_MCU_8u16s_C4P4R)(pBuf,8*4,pDst);
  }

  return;
} /* RGBA_FPX_to_YCbCrA_FPX_1111_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    RGBA_FPX_to_YCbCrA_FPX_4224_MCU
//
//  Purpose
//    Color convert a MCU of 4-channel RGBA_FPX DIB input data into
//    4:2:2:4 YCbCrA_FPX format.
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) RGBA_FPX_to_YCbCrA_FPX_4224_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 8;
  int     bytes      = 4;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*8*4+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[4];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C4R(inptr,lineoffset,realSize,pBuf,16*4,wishSize);
    }
    else
    {
      IppiSize realSizeC1;

      realSizeC1.width  = pixwidth*4;
      realSizeC1.height = pixheight;

      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1R)(inptr,lineoffset,pBuf,16*4,realSizeC1,ippAxsHorizontal);
      ippiExpand_8u_C4IR(pBuf,16*4,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*2];
    pDst[2] = &outptr[DCTSIZE2*3];
    pDst[3] = &outptr[DCTSIZE2*4];

    IPPCALL(ippiCMYKToYCCK422LS_MCU_8u16s_C4P4R)(pBuf,16*4,pDst);
  }

  return;
} /* RGBA_FPX_to_YCbCrA_FPX_4224_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    RGBA_FPX_to_YCbCrA_FPX_4114_MCU
//
//  Purpose
//    Color convert a MCU of 4-channel RGBA_FPX DIB input data into
//    4:1:1:4 YCbCrA_FPX format.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) RGBA_FPX_to_YCbCrA_FPX_4114_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 16;
  int     bytes      = 4;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*16*4+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[4];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C4R(inptr,lineoffset,realSize,pBuf,16*4,wishSize);
    }
    else
    {
      IppiSize realSizeC1;

      realSizeC1.width  = pixwidth*4;
      realSizeC1.height = pixheight;

      lineoffset = -lineoffset;
      inptr -= (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1R)(inptr,lineoffset,pBuf,16*4,realSizeC1,ippAxsHorizontal);
      ippiExpand_8u_C4IR(pBuf,16*4,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*4];
    pDst[2] = &outptr[DCTSIZE2*5];
    pDst[3] = &outptr[DCTSIZE2*6];

    IPPCALL(ippiCMYKToYCCK411LS_MCU_8u16s_C4P4R)(pBuf,16*4,pDst);
  }

  return;
} /* RGBA_FPX_to_YCbCrA_FPX_4114_MCU() */


/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippiSampleDown444LS_MCU_8u16s_C4P4R
//
//  Purpose:
//    down-sampling functions and
//    level shift (-128) for one MCU
//
//  Parameter:
//    pSrc      pointer to input data
//    srcStep   line offset in input data
//    pDst      pointer to pointers to output data
//
//  Returns:
//    IppStatus
//
//  Notes:
*/

LOCFUN(IppStatus) ippiSampleDown444LS_MCU_8u16s_C4P4R(
  const Ipp8u*  pSrc,
        int     srcStep,
        Ipp16s* pDst[4])
{
  int i;
  Ipp16s* Ch1;
  Ipp16s* Ch2;
  Ipp16s* Ch3;
  Ipp16s* Ch4;

  Ch1 = pDst[0];
  Ch2 = pDst[1];
  Ch3 = pDst[2];
  Ch4 = pDst[3];

  for(i = 0; i < 8; i++)
  {
    Ch1[0] = (Ipp16s)(pSrc[ 0] - 128);
    Ch2[0] = (Ipp16s)(pSrc[ 1] - 128);
    Ch3[0] = (Ipp16s)(pSrc[ 2] - 128);
    Ch4[0] = (Ipp16s)(pSrc[ 3] - 128);

    Ch1[1] = (Ipp16s)(pSrc[ 4] - 128);
    Ch2[1] = (Ipp16s)(pSrc[ 5] - 128);
    Ch3[1] = (Ipp16s)(pSrc[ 6] - 128);
    Ch4[1] = (Ipp16s)(pSrc[ 7] - 128);

    Ch1[2] = (Ipp16s)(pSrc[ 8] - 128);
    Ch2[2] = (Ipp16s)(pSrc[ 9] - 128);
    Ch3[2] = (Ipp16s)(pSrc[10] - 128);
    Ch4[2] = (Ipp16s)(pSrc[11] - 128);

    Ch1[3] = (Ipp16s)(pSrc[12] - 128);
    Ch2[3] = (Ipp16s)(pSrc[13] - 128);
    Ch3[3] = (Ipp16s)(pSrc[14] - 128);
    Ch4[3] = (Ipp16s)(pSrc[15] - 128);

    Ch1[4] = (Ipp16s)(pSrc[16] - 128);
    Ch2[4] = (Ipp16s)(pSrc[17] - 128);
    Ch3[4] = (Ipp16s)(pSrc[18] - 128);
    Ch4[4] = (Ipp16s)(pSrc[19] - 128);

    Ch1[5] = (Ipp16s)(pSrc[20] - 128);
    Ch2[5] = (Ipp16s)(pSrc[21] - 128);
    Ch3[5] = (Ipp16s)(pSrc[22] - 128);
    Ch4[5] = (Ipp16s)(pSrc[23] - 128);

    Ch1[6] = (Ipp16s)(pSrc[24] - 128);
    Ch2[6] = (Ipp16s)(pSrc[25] - 128);
    Ch3[6] = (Ipp16s)(pSrc[26] - 128);
    Ch4[6] = (Ipp16s)(pSrc[27] - 128);

    Ch1[7] = (Ipp16s)(pSrc[28] - 128);
    Ch2[7] = (Ipp16s)(pSrc[29] - 128);
    Ch3[7] = (Ipp16s)(pSrc[30] - 128);
    Ch4[7] = (Ipp16s)(pSrc[31] - 128);

    /* Advance the output pointers to Ch1, Ch2, Ch3 and Ch4. */
    Ch1 += 8;
    Ch2 += 8;
    Ch3 += 8;
    Ch4 += 8;

    /* Go to the next line. */
    pSrc += srcStep;
  }

  return ippStsNoErr;
} /* ippiSampleDown444LS_MCU_8u16s_C4P4R() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_to_OTHER_1111_MCU
//
//  Purpose
//    Move a MCU of 4-channel RGBA_FPX DIB input data into
//    1:1:1:1 RGBA_FPX format.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_to_OTHER_1111_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 8;
  int     y          = 8;
  int     bytes      = 4;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[8*8*4+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[4];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C4R(inptr,lineoffset,realSize,pBuf,8*4,wishSize);
    }
    else
    {
      IppiSize realSizeC1;

      realSizeC1.width  = pixwidth*4;
      realSizeC1.height = pixheight;

      lineoffset = -lineoffset;
      inptr -=(pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1R)(inptr,lineoffset,pBuf,8*4,realSizeC1,ippAxsHorizontal);
      ippiExpand_8u_C4IR(pBuf,8*4,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*1];
    pDst[2] = &outptr[DCTSIZE2*2];
    pDst[3] = &outptr[DCTSIZE2*3];

    ippiSampleDown444LS_MCU_8u16s_C4P4R(pBuf,8*4,pDst);
  }

  return;
} /* OTHER_to_OTHER_1111_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_to_OTHER_111_MCU
//
//  Purpose
//    rewrites input 3-channels data into MCU blocks without subsampling and
//    color conversion.
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_to_OTHER_111_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 8;
  int     y          = 8;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[8*8*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,8*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -=(pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,8*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,8*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*1];
    pDst[2] = &outptr[DCTSIZE2*2];

    IPPCALL(ippiSampleDown444LS_MCU_8u16s_C3P3R)(pBuf,8*3,pDst);
  }

  return;
} /* OTHER_to_OTHER_111_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_to_OTHER_411_MCU
//
//  look
//    CC_NO_SS_111 for details
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_to_OTHER_411_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 16;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*16*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,16*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -=(pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,16*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,16*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*4];
    pDst[2] = &outptr[DCTSIZE2*5];

    IPPCALL(ippiSampleDown411LS_MCU_8u16s_C3P3R)(pBuf,16*3,pDst);
  }

  return;
} /* OTHER_to_OTHER_411_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_to_OTHER_422_MCU
//  look
//    CC_NO_SS_111 for details
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_to_OTHER_422_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 8;
  int     bytes      = 3;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*8*3+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C3R(inptr,lineoffset,realSize,pBuf,16*3,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -=(pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(inptr,lineoffset,pBuf,16*3,realSize,ippAxsHorizontal);
      ippiExpand_8u_C3IR(pBuf,16*3,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*2];
    pDst[2] = &outptr[DCTSIZE2*3];

    IPPCALL(ippiSampleDown422LS_MCU_8u16s_C3P3R)(pBuf,16*3,pDst);
  }

  return;
} /* OTHER_to_OTHER_422_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbYCr_to_YCbCr_422_MCU
//
//  Purpose
//    Convert |Y|Cb|Y|Cr| input data into 4:2:2 |Y|Cb|Cr| format.
//  This routine is a C implementation.
//
//  look CC_NO_SS_422 for details.
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbYCr_to_YCbCr_422_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 16;
  int     y          = 8;
  int     bytes      = 2;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[16*8*2+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE-1);
    Ipp16s* pDst[3];

    realSize.width  = pixwidth*2;
    realSize.height = pixheight;

    wishSize.width  = x*2;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C1R(inptr,lineoffset,realSize,pBuf,16*2,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr      = inptr - (pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1R)(inptr,lineoffset,pBuf,16*2,realSize,ippAxsHorizontal);
      ippiExpand_8u_C1IR(pBuf,16*2,realSize,wishSize);
    }

    pDst[0] = &outptr[DCTSIZE2*0];
    pDst[1] = &outptr[DCTSIZE2*2];
    pDst[2] = &outptr[DCTSIZE2*3];

    IPPCALL(ippiSplit422LS_MCU_8u16s_C2P3R)(pBuf,16*2,pDst);
  }

  return;
} /* YCbYCr_to_YCbCr_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Y_to_Y_111_MCU
//
//  Purpose
//    rewrites input 1-channels data into MCU blocks without subsampling and
//    color conversion.
//
//    This routine is a C implementation.
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) Y_to_Y_111_MCU(
  JPEG_PROPERTIES*  jprops,
  int               curxMCU,
  int               curyMCU)
{
  int     testpix;
  int     x          = 8;
  int     y          = 8;
  int     bytes      = 1;
  int     pixwidth   = x;
  int     pixheight  = y;
  Ipp8u*  inptr      = jprops->state.DIB_ptr;
  int     lineoffset = jprops->DIBLineBytes;
  Ipp16s* outptr     = (Ipp16s*)jprops->MCUBuf;
  Ipp8u   tmp[8*8*1+CPU_CACHE_LINE-1];

  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* last MCU in a row of MCUs */
    testpix = jprops->DIBWidth % x;

    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (y * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * bytes);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* last row of MCUs in the image */
    testpix = abs(jprops->DIBHeight) % y;

    if(testpix)
    {
      pixheight = testpix;
    }
  }

  {
    IppiSize realSize;
    IppiSize wishSize;
    Ipp8u* pBuf = (Ipp8u*)OWN_ALIGN_PTR(tmp,CPU_CACHE_LINE);

    realSize.width  = pixwidth;
    realSize.height = pixheight;

    wishSize.width  = x;
    wishSize.height = y;

    if(lineoffset > 0)
    {
      ippiCopyExpand_8u_C1R(inptr,lineoffset,realSize,pBuf,8,wishSize);
    }
    else
    {
      lineoffset = -lineoffset;
      inptr -=(pixheight - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1R)(inptr,lineoffset,pBuf,8,realSize,ippAxsHorizontal);
      ippiExpand_8u_C1IR(pBuf,8,realSize,wishSize);
    }

    IPPCALL(ippiSub128_JPEG_8u16s_C1R)(pBuf,8,outptr,8*sizeof(Ipp16s),wishSize);
  }

  return;
} /* Y_to_Y_111_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
// Name
//   CC_SS_RGB_to_YCbCr_General_MCU
//
// Purpose
//   Color convert and downsample from RGB input data to YCbCr using equations defined
//   in the JFIF format standard.
//
//   This routine expects 3 or 4 channel input DIBs
//   and produces 3 or 4 channel JPEG images.
//
// Note
//   in this routine, downsampling is "pure sampling"
//   (i.e., no averaging over multiple pixels is done).
//
// Context
//   Works on a single MCU of data. Also, takes care of data padding.
//
// Returns
//   none
//
// Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) CC_SS_RGB_to_YCbCr_General_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int    i, j, k, je, ke;
  int    h, v, iv, ih, h_expand, v_expand;
  int    pixwidth, pixheight, numpix, numpix2;
  int    testpix;
  int    r, g, b;
  short  inpR, inpG, inpB;
  Ipp8u* inptr;
  Ipp8u* tempinptr;
  short* outptr;
  short* temptr;
  short* temMCUBuf;
  Ipp8u  tempBuff[MAX_MCU_SIZE*2*sizeof(short) + CPU_CACHE_LINE];


  temMCUBuf = (short*)OWN_ALIGN_PTR(&tempBuff[0],CPU_CACHE_LINE);

  /* Assume all upsampling is separable and assume MCUs are */
  /* all in the same scan (no progressive or hierarchical). */
  outptr = (short*)jprops->MCUBuf;

  /*
  // Offset from the current (x,y) pixel location in the
  // output buffer is determined by the index of the
  // current scan component.
  */
  inptr = jprops->state.DIB_ptr;

  pixwidth  = 8 * jprops->jframe.max_hsampling;
  pixheight = 8 * jprops->jframe.max_vsampling;

  /* Select the start of the next output buffer group. */
  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* Last MCU in the row. */
    testpix = jprops->DIBWidth % pixwidth;
    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes +
      (8 * jprops->jframe.max_vsampling * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * jprops->DIBChannels);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* Last line of the image. */
    testpix = abs(jprops->DIBHeight) % pixheight;
    if(testpix)
    {
      pixheight = testpix;
    }
  }

  /* Separate input data into planes and pad if needed. */
  temptr = temMCUBuf;
  for(i = 0; i < jprops->JPGChannels; i++)
  {
    tempinptr = inptr + i;

    for(j = 0; j < (pixheight * 16); j += 16)
    {
      for(k = 0; k < pixwidth; k++)
      {
        temptr[j + k] = (short)tempinptr[k * jprops->DIBChannels];
      }

      for(k = pixwidth; k < (8 * jprops->jframe.max_hsampling); k++)
      {
        temptr[j + k] = (short)temptr[j + pixwidth-1];
      }

      tempinptr += jprops->DIBLineBytes;
    }

    for(j = (pixheight * 16); j < (8 * jprops->jframe.max_vsampling * 16); j += 16)
    {
      for(k = 0; k < (8 * jprops->jframe.max_hsampling); k++)
      {
        temptr[j + k] = temptr[(pixheight * 16) - 16 + k];
      }
    }

    temptr += 512;
  }

  /* color convert to Y only. */
  je = 128 * jprops->jframe.max_vsampling;
  ke = 8 * jprops->jframe.max_hsampling;
  for(j = 0; j < je; j += 128)
  {
    for(k = 0; k < ke; k += 8)
    {
      for(v = 0; v < 128; v += 16)
      {
        for(h = 0; h < 8; h ++)
        {
          if(jprops->DIBColor == IJL_RGB)
          {
            inpR = temMCUBuf[j + k + v + h];
            inpG = temMCUBuf[j + k + v + h + 512 ],
            inpB = temMCUBuf[j + k + v + h + 1024];
          }
          else
          {
            inpR = (short)(255 - temMCUBuf[j + k + v + h]);
            inpG = (short)(255 - temMCUBuf[j + k + v + h + 512 ]),
            inpB = (short)(255 - temMCUBuf[j + k + v + h + 1024]);
          }
          *outptr++ = (short)CC_RGB_Y(inpR, inpG, inpB);
        }
      }
    }
  }

  /* accumulate neighbour pixels, color convert to Cb and subsampling */
  h_expand = jprops->jframe.max_hsampling / jprops->jframe.comps[1].hsampling;
  v_expand = jprops->jframe.max_vsampling / jprops->jframe.comps[1].vsampling;
  numpix   = h_expand * v_expand;
  numpix2  = (h_expand * v_expand) >> 1;

  for(j = 0; j < je; j += (128 * v_expand))
  {
    for(k = 0; k < ke; k += (8 * h_expand))
    {
      for(v = 0; v < (128 * v_expand); v += (v_expand * 16))
      {
        for(h = 0; h < (8 * h_expand); h += h_expand)
        {
          r = 0;
          g = 0;
          b = 0;
          for(iv = 0; iv < (v_expand * 16); iv += 16)
          {
            for(ih = 0; ih < h_expand; ih++)
            {
              r += temMCUBuf[j + k + v + h + iv + ih];
              g += temMCUBuf[j + k + v + h + iv + ih + 512 ];
              b += temMCUBuf[j + k + v + h + iv + ih + 1024];
            }
          }
          inpR = (short)((r + numpix2) / numpix);
          inpG = (short)((g + numpix2) / numpix);
          inpB = (short)((b + numpix2) / numpix);
          if(jprops->DIBColor != IJL_RGB)
          {
            inpR = (short)(255 - inpR);
            inpG = (short)(255 - inpG);
            inpB = (short)(255 - inpB);
          }
          *outptr++ = (short)CC_RGB_CB(inpR, inpG, inpB);
        }
      }
    }
  }

  /* accumulate neighbour pixels, color convert to Cr and subsampling */
  h_expand = jprops->jframe.max_hsampling / jprops->jframe.comps[2].hsampling;
  v_expand = jprops->jframe.max_vsampling / jprops->jframe.comps[2].vsampling;
  numpix   = h_expand * v_expand;
  numpix2  = (h_expand * v_expand) >> 1;

  for(j = 0; j < je; j += (128 * v_expand))
  {
    for(k = 0; k < ke; k += (8 * h_expand))
    {
      for(v = 0; v < (128 * v_expand); v += (v_expand * 16))
      {
        for(h = 0; h < (8 * h_expand); h += h_expand)
        {
          r = 0;
          g = 0;
          b = 0;
          for(iv = 0; iv < (v_expand * 16); iv += 16)
          {
            for(ih = 0; ih < h_expand; ih++)
            {
              r += temMCUBuf[j + k + v + h + iv + ih];
              g += temMCUBuf[j + k + v + h + iv + ih + 512 ];
              b += temMCUBuf[j + k + v + h + iv + ih + 1024];
            }
          }
          inpR = (short)((r + numpix2) / numpix);
          inpG = (short)((g + numpix2) / numpix);
          inpB = (short)((b + numpix2) / numpix);
          if(jprops->DIBColor != IJL_RGB)
          {
            inpR = (short)(255 - inpR);
            inpG = (short)(255 - inpG);
            inpB = (short)(255 - inpB);
          }
          *outptr++ = (short)CC_RGB_CR(inpR, inpG, inpB);
        }
      }
    }
  }

  if(jprops->DIBColor != IJL_RGB) /* RGBA */
  {
    /* accumulate neighbour pixels, "flip" and subsampling */
    h_expand = jprops->jframe.max_hsampling / jprops->jframe.comps[3].hsampling;
    v_expand = jprops->jframe.max_vsampling / jprops->jframe.comps[3].vsampling;
    numpix   = h_expand * v_expand;
    numpix2  = (h_expand * v_expand) >> 1;
    for(j = 0; j < je; j += (128 * v_expand))
    {
      for(k = 0; k < ke; k += (8 * h_expand))
      {
        for(v = 0; v < (128 * v_expand); v += (v_expand * 16))
        {
          for(h = 0; h < (8 * h_expand); h += h_expand)
          {
            r = 0;
            for(iv = 0; iv < (v_expand * 16); iv += 16)
            {
              for(ih = 0; ih < h_expand; ih++)
              {
                r += temMCUBuf[j + k + v + h + iv + ih + 1536];
              }
            }
            if(jprops->DIBColor == IJL_RGB)
            {
              *outptr++ = (short)((r + numpix2) / numpix);
            }
            else
            {
              *outptr++ = (short)((r + numpix2) / numpix - 128);
            }
          }
        }
      }
    }
  }

  return;
} /* CC_SS_RGB_to_YCbCr_General_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
// Name
//   CC_RGB_to_YCbCr_General_MCU
//
// Purpose
//   General color conversion of RGB->YCbCr
//   as defined in the JFIF format standard.
//
// Context
//   Works on a single MCU at a time.
//
// Returns
//   none
//
// Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) CC_RGB_to_YCbCr_General_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int     i, j, k;
  int     testpix;
  int     pixwidth, pixheight;
  short   inpR, inpG, inpB;
  Ipp8u*  inptr;
  short*  outptr;
  short*  temptr;

  /* Assume all upsampling is separable and assume MCUs are */
  /* all in the same scan (no progressive or hierarchical). */
  outptr = (short*)jprops->MCUBuf;

  /* Offset from the current (x,y) pixel location in the output buffer */
  /* is determined by the index of the current scan component.         */
  inptr = jprops->state.DIB_ptr;

  pixwidth  = 8;
  pixheight = 8;

  /* Select the start of the next output buffer group. */
  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* Last MCU in the row. */
    testpix = jprops->DIBWidth % 8;
    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (8 * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (8 * jprops->DIBChannels);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* Last line of the image. */
    testpix = abs(jprops->DIBHeight) % 8;
    if(testpix)
    {
      pixheight = testpix;
    }
  }

  if((jprops->JPGColor == IJL_G) &&
    (jprops->DIBColor == IJL_RGB || jprops->DIBColor == IJL_BGR))
  {
    /* Color convert the DIB data (in interleaved format) into the */
    /* output buffer (a single luminance plane). */
    for(j = 0; j < pixheight; j++)
    {
      for(k = 0; k < pixwidth; k++)
      {
        if(jprops->DIBColor == IJL_RGB)
        {
          inpR = inptr[(j * jprops->DIBLineBytes) + (k * jprops->DIBChannels) + 0];
          inpG = inptr[(j * jprops->DIBLineBytes) + (k * jprops->DIBChannels) + 1];
          inpB = inptr[(j * jprops->DIBLineBytes) + (k * jprops->DIBChannels) + 2];
        }
        else /* (jprops->DIBColor == IJL_BGR) */
        {
          inpB = inptr[(j * jprops->DIBLineBytes) + (k * jprops->DIBChannels) + 0];
          inpG = inptr[(j * jprops->DIBLineBytes) + (k * jprops->DIBChannels) + 1];
          inpR = inptr[(j * jprops->DIBLineBytes) + (k * jprops->DIBChannels) + 2];
        }

        outptr[(j * 8) + k] = (short)CC_RGB_Y(inpR, inpG, inpB);
      }
      for(k = pixwidth; k < 8; k++)
      {
        outptr[(j * 8) + k] = outptr[(j * 8) + pixwidth - 1];
      }
    }
    for(j = (pixheight * 8); j < DCTSIZE2; j += 8)
    {
      for(k = 0; k < 8; k++)
      {
        outptr[j + k] = outptr[(pixheight * 8) + k - 8];
      }
    }
  }
  else
  {
    /* Copy the DIB data from an interleaved format into a */
    /* planar format in the output buffer.                 */
    temptr = outptr;
    for(i = 0; i < jprops->JPGChannels; i++)
    {
      for(j = 0; j < pixheight; j++)
      {
        for(k = 0; k < pixwidth; k++)
        {
          temptr[(j * 8) + k] = inptr[(k * jprops->DIBChannels) + (j * jprops->DIBLineBytes)];
        }
        for(k = pixwidth; k < 8; k++)
        {
          temptr[(j * 8) + k] = temptr[(j * 8) + pixwidth - 1];
        }
      }
      for(j = (pixheight * 8); j < DCTSIZE2; j += 8)
      {
        for(k = 0; k < 8; k++)
        {
          temptr[j + k] = temptr[(pixheight * 8) + k - 8];
        }
      }
      temptr += DCTSIZE2;
      inptr++;
    }

    /* Color convert the entire input MCU. */
    for(j = 0; j < DCTSIZE2; j += 8)
    {
      for(k = 0; k < 8; k++)
      {
        if(jprops->DIBColor == IJL_BGR)
        {
          inpB = outptr[j + k      ];
          inpG = outptr[j + k +  64];
          inpR = outptr[j + k + 128];
        }
        else if(jprops->DIBColor == IJL_RGB)
        {
          inpR = outptr[j + k      ];
          inpG = outptr[j + k +  64];
          inpB = outptr[j + k + 128];
        }
        else /* (jprops->DIBColor == IJL_RGBA_FPX) */
        {
          /* Do the FlashPix "flip" where X' = 255 - X. */
          inpR = (short)(255 - outptr[j + k      ]);
          inpG = (short)(255 - outptr[j + k +  64]);
          inpB = (short)(255 - outptr[j + k + 128]);
          /* -128 is the level shift to prepare for fDCT. */
          outptr[j + k + 192] = (short)(outptr[j + k + 192] - 128);
        }

        outptr[j + k      ] = (short)CC_RGB_Y(inpR, inpG, inpB);
        outptr[j + k +  64] = (short)CC_RGB_CB(inpR, inpG, inpB);
        outptr[j + k + 128] = (short)CC_RGB_CR(inpR, inpG, inpB);
      }
    }
  }

  return;
} /* CC_RGB_to_YCbCr_General_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
// Name
//   SS_General_MCU
//
// Purpose
//   Performs supported downsampling on n-channel DIB images
//   into JPEG-compliant subsampling ratios (1-4 channels).
//
// Note
//   in this routine, downsampling is "pure sampling"
//   (i.e., no averaging over multiple pixels is done).
//
// Context
//   Works on a single MCU of data at a time.
//
// Returns
//   none
//
// Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) SS_General_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int    i, j, k;
  int    h, v, h_expand, v_expand, iv, ih, numpix, numpix2;
  int    pixwidth, pixheight;
  int    testpix;
  Ipp8u* inptr;
  Ipp8u* tempinptr;
  short* outptr;
  Ipp8u  temMCUBuf[MAX_MCU_SIZE*sizeof(short)];

  /* Assume all upsampling is separable and assume MCUs are */
  /* all in the same scan (no progressive or hierarchical). */
  outptr = (short*)jprops->MCUBuf;

  /* Offset from the current (x,y) pixel location in the output buffer */
  /* is determined by the index of the current scan component.         */
  inptr = jprops->state.DIB_ptr;

  pixwidth  = 8 * jprops->jframe.max_hsampling;
  pixheight = 8 * jprops->jframe.max_vsampling;

  /* Select the start of the next output buffer group. */
  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* Last MCU in the row. */
    testpix = jprops->DIBWidth % pixwidth;
    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes +
      (8 * jprops->jframe.max_vsampling * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (pixwidth * jprops->DIBChannels);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* Last line of the image. */
    testpix = abs(jprops->DIBHeight) % pixheight;
    if(testpix)
    {
      pixheight = testpix;
    }
  }

  /* Separate input data into planes, pad any extra data. */
  for(i = 0; i < jprops->JPGChannels; i++)
  {
    if((i == 0) || (jprops->DIBColor != IJL_G) || (jprops->JPGColor != IJL_YCBCR))
    {
      if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
      {
        tempinptr = inptr + 2 - i;
      }
      else
      {
        tempinptr = inptr + i;
      }

      for(j = 0; j < (pixheight * 16); j += 16)
      {
        for(k = 0; k < pixwidth; k++)
        {
          temMCUBuf[j + k] = tempinptr[k * jprops->DIBChannels];
        }
        for(k = pixwidth; k < (8 * jprops->jframe.max_hsampling); k++)
        {
          temMCUBuf[j + k] = temMCUBuf[j + pixwidth - 1];
        }

        tempinptr += jprops->DIBLineBytes;
      }

      for(j = (pixheight * 16); j < (8 * jprops->jframe.max_vsampling * 16); j += 16)
      {
        for(k = 0; k < (8 * jprops->jframe.max_hsampling); k++)
        {
          temMCUBuf[j + k] = temMCUBuf[(pixheight * 16) - 16 + k];
        }
      }
    }

    h_expand = jprops->jframe.max_hsampling / jprops->jframe.comps[i].hsampling;
    v_expand = jprops->jframe.max_vsampling / jprops->jframe.comps[i].vsampling;
    numpix   = h_expand * v_expand;
    numpix2  = (h_expand * v_expand) >> 1;

    for(j = 0; j < (8 * jprops->jframe.max_vsampling * 16); j += (8 * v_expand * 16))
    {
      for(k = 0; k < (8 * jprops->jframe.max_hsampling); k += (8 * h_expand))
      {
        for(v = 0; v < (8 * v_expand * 16); v += (v_expand * 16))
        {
          for(h = 0; h < (8 * h_expand); h += h_expand)
          {
            *outptr = 0;
            if((i == 0) || (jprops->DIBColor != IJL_G) || (jprops->JPGColor != IJL_YCBCR))
            {
              for(iv = 0; iv < (v_expand * 16); iv += 16)
              {
                for(ih = 0; ih < h_expand; ih++)
                {
                  *outptr = (short)(*outptr + temMCUBuf[j + k + v + h + iv + ih]);
                }
              }

              /* -128 is the level shift to prepare for fDCT. */
              *outptr = (short)(((*outptr) + numpix2) / numpix - 128);
            }

            /*
            // For JPEG channels Cb and Cr of Grayscale->YCbCr,
            // fill up w/ values that mean no color/chrominance
            // (i.e., accounting for the level shift to prepare
            // for the fDCT, we need values of 128 - 128 = 0).
            */
            outptr++;
          }
        }
      }
    }
  }

  return;
} /* SS_General_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
// Name
//   Input_Interleave_General_MCU
//
// Purpose
//   Interleaves data from the input DIB into the MCU Buffer.
//   No downsampling or color conversion is performed.
//
// Context
//   Works on a single MCU of data at a time.
//
// Returns
//   none
//
// Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) Input_Interleave_General_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int            i, j, k;
  int            pixwidth, pixheight;
  int            testpix;
  short*         outptr;
  unsigned char* inptr;
  unsigned char* temptr;

  /* Assume all upsampling is separable and assume MCUs are all */
  /* in the same scan (no progressive or hierarchical). */

  outptr = (short*)jprops->MCUBuf;

  /*
  // Offset from the current (x,y) pixel location in the
  // output buffer is determined by the index of the current
  // scan component.
  */
  inptr = jprops->state.DIB_ptr;

  pixwidth  = 8;
  pixheight = 8;

  /* Select the start of the next output buffer group. */
  if(curxMCU == (jprops->numxMCUs - 1))
  {
    /* Last MCU in the row. */
    testpix = jprops->DIBWidth % 8;
    if(testpix)
    {
      pixwidth = testpix;
    }

    jprops->state.DIB_ptr = jprops->DIBBytes + (8 * jprops->DIBLineBytes * (curyMCU + 1));
  }
  else
  {
    jprops->state.DIB_ptr += (8 * jprops->DIBChannels);
  }

  if(curyMCU == (jprops->numyMCUs - 1))
  {
    /* Last line of the image. */
    testpix = abs(jprops->DIBHeight) % 8;
    if(testpix)
    {
      pixheight = testpix;
    }
  }

  /* Separate input data into planes and pad if needed. */
  for(i = 0; i < jprops->JPGChannels; i++)
  {
    if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
    {
      temptr = inptr + 2 - i;
    }
    else
    {
      temptr = inptr + i;
    }

    for(j = 0; j < pixheight; j++)
    {
      for(k = 0; k < pixwidth; k++)
      {
        if((i == 0) || (jprops->DIBColor != IJL_G) || (jprops->JPGColor != IJL_YCBCR))
        {
          /* -128 is the level shift to prepare for fDCT. */
          outptr[(j * 8) + k] = (short)(temptr[k * jprops->DIBChannels] - 128);
        }
        else
        {
          /*
          // For JPEG channels Cb and Cr of Grayscale->YCbCr,
          // fill up w/ values that mean no color/chrominance
          // (i.e., accounting for the level shift to prepare
          // for the fDCT, we need values of 128 - 128 = 0).
          */
          outptr[(j * 8) + k] = 0;
        }
      }
      for(k = pixwidth; k < 8; k++)
      {
        outptr[(j * 8) + k] = outptr[(j * 8) + pixwidth - 1];
      }

      temptr += jprops->DIBLineBytes;
    }
    for(j = (pixheight * 8); j < DCTSIZE2; j += 8)
    {
      for(k = 0; k < 8; k++)
      {
        outptr[j + k] = outptr[(pixheight * 8) + k - 8];
      }
    }
    outptr += DCTSIZE2;
  }

  return;
} /* Input_Interleave_General_MCU() */

