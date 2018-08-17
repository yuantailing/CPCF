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
#ifndef __ENCODER_PARSER_H__
#include "parser/encoder_parser.h"
#endif



#ifdef __INTEL_COMPILER
#pragma warning(disable:174)
#pragma warning(disable:424)
#endif



//EXC_INIT();


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_APP0
//
//  Purpose
//    Write a JFIF APP0 marker. This is the first marker to follow
//    a SOI marker in the beginning of a JFIF compliant JPEG stream.
//    It allows for such niceties as a concept of equivalent dimension
//    (X pixels = how many inches?) and thumbnails. It is required for
//    JFIF images.
//
//  Context
//
//  Returns
//    A valid error code, or 0 if no error.
//
//  Parameters
//    version    - version of JFIF format (0x0101)
//    units      - units for X and Y densities,
//                   0 - no units, X and Y specify pixel aspect ratio
//                   1 - X and Y are dots per inch
//                   2 - X and Y are dots per cm
//    Xdensity   - horizontal pixel density
//    Ydensity   - vertical pixel density
//    pThumbnail - Pointer to the thumnail data (or NULL if none)
//    thWidth    - thumnail width (or 0 if none)
//    thHeight   - thumnail height (or 0 if none)
//    pState     - Pointer to the current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_APP0(
  int    version,
  int    units,
  int    Xdensity,
  int    Ydensity,
  Ipp8u* pThumbnail,
  int    thWidth,
  int    thHeight,
  STATE* pState)
{
  int    i;
  int    len;
  int    thSize;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_APP0()\n");

  jerr = _WRITE_WORD(0xff00 | MARKER_APP0,pState);
  if(IJL_OK != jerr)
    goto Exit;

  thSize = 3*thWidth*thHeight;

  len = 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1 + thSize;

  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* write identifier "JFIF\0" */
  jerr = _WRITE_BYTE('J',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('F',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('I',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('F',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE(0,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* JFIF version */
  jerr = _WRITE_WORD(version,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* aspect ratio units */
  jerr = _WRITE_BYTE(units,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(Xdensity,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(Ydensity,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* thumbnails dimensions */
  jerr = _WRITE_BYTE(thWidth,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE(thHeight,pState);
  if(IJL_OK != jerr)
    goto Exit;

  if(thSize != 0 && pThumbnail != 0)
  {
    for(i = 0; i < thSize; i++)
    {
      jerr = _WRITE_BYTE(pThumbnail[i],pState);
      if(IJL_OK != jerr)
        goto Exit;
    }
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_APP0()\n");

  return jerr;
} /* EP_Write_APP0() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_COM
//
//  Purpose
//    Write a JFIF COM marker.
//
//  Context
//
//  Returns
//    A valid error code, or 0 if no error.
//
//  Parameters
//    pData      The pointer to comment data
//    nDataBytes Number of data bytes
//    pState     The current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_COM(
  Ipp8u* pData,
  int    nDataBytes,
  STATE* pState)
{
  int    i;
  int    len;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_COM()\n");

  TRY_BEGIN

  if(NULL != pData && 0 != nDataBytes)
  {
    nDataBytes = IPP_MIN(nDataBytes,MAX_COMMENTS_SIZE);

    /*
    // length of comment segment consist from
    //   2          - COM marker length
    //   2          - field length in COM segment
    //   nDataBytes - length of user comment
    */
    len = 2 + 2 + nDataBytes;

    /* write marker COM to temporary buffer */
    jerr = _WRITE_WORD(0xff00 | MARKER_COM,pState);
    if(IJL_OK != jerr)
      goto Exit;

    /*
    // write length parameter
    // NOTE:
    //   this not included length COM marker code, but include length of
    //   field itself
    */

    jerr = _WRITE_WORD(len - 2,pState);
    if(IJL_OK != jerr)
      goto Exit;

    /* write comment to JPEG stream */
    for(i = 0; i < nDataBytes; i++)
    {
      jerr = _WRITE_BYTE(pData[i],pState);
      if(IJL_OK != jerr)
        goto Exit;
    }

    TRACE1(trINFO,"comment: %s\n",pData);
  }

  TRY_END

  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: Exception detected\n");
  jerr = IJL_ERR_COM_BUFFER;

  CATCH_END

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_COM()\n");

  return jerr;
} /* EP_Write_COM() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_SOI
//
//  Purpose
//    Write an start-of-image marker.
//
//  Context
//
//  Returns
//    a valid error code, or 0 if ok
//
//  Parameters
//    STATE* pState  Pointer to the current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_SOI(
  STATE* pState)
{
  IJLERR jerr;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_SOI()\n");

  jerr = _WRITE_WORD(0xff00 | MARKER_SOI,pState);
  if(IJL_OK != jerr)
  {
    TRACE1(trERROR,"ERROR: _WRITE_WORD() failed - %d\n",jerr);
  }

  TRACE0(trCALL|trAFTER,"leave EP_Write_SOI()\n");

  return jerr;
} /* EP_Write_SOI() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_EOI
//
//  Purpose
//    Write an End-Of-Image marker (EOI).
//
//  Context
//
//  Returns
//    a valid error code, or 0 if OK
//
//  Parameters
//    STATE* pState  Pointer to the current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_EOI(
  STATE* pState)
{
  IJLERR jerr;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_EOI()\n");

  jerr = _WRITE_WORD(0xff00 | MARKER_EOI,pState);
  if(IJL_OK != jerr)
  {
    TRACE1(trERROR,"ERROR: _WRITE_WORD() failed - %d\n",jerr);
  }

  TRACE0(trCALL|trAFTER,"leave EP_Write_EOI()\n");

  return jerr;
} /* EP_Write_EOI() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_SOF
//
//  Purpose
//    Write a SOF (Start-Of-Frame) segment.
//
//  Context
//
//  Returns
//    a valid error code, or 0 if OK
//
//  Parameters
//    pFrame  The pointer to JPEG frame structure
//    pState  The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_SOF(
  FRAME* pFrame,
  STATE* pState)
{
  int    i;
  int    len;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_SOF()\n");

  len = 8 + (pFrame->ncomps * 3);

  if(pFrame->ncomps < 0 || pFrame->ncomps > 4)
  {
    jerr = IJL_UNSUPPORTED_FRAME;
    goto Exit;
  }

  jerr = _WRITE_WORD(0xff00 | MARKER_SOF0,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  if(pFrame->precision != 8)
  {
    jerr = IJL_UNSUPPORTED_FRAME;
    goto Exit;
  }

  /* sample precision */
  jerr = _WRITE_BYTE(pFrame->precision,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* JPEG image height */
  jerr = _WRITE_WORD(pFrame->height,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* JPEG image width */
  jerr = _WRITE_WORD(pFrame->width,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* number of channels */
  jerr = _WRITE_BYTE(pFrame->ncomps,pState);
  if(IJL_OK != jerr)
    goto Exit;

  for(i = 0; i < pFrame->ncomps; i++)
  {
    int sampling;

    /* component id */
    jerr = _WRITE_BYTE(pFrame->comps[i].ident,pState);
    if(IJL_OK != jerr)
      goto Exit;

    /* component sampling factors */
    sampling = (pFrame->comps[i].hsampling << 4) + pFrame->comps[i].vsampling;

    if(pFrame->ncomps == 1 && sampling != 0x11)
    {
      jerr = IJL_UNSUPPORTED_SUBSAMPLING;
      goto Exit;
    }

    jerr = _WRITE_BYTE(sampling,pState);
    if(IJL_OK != jerr)
      goto Exit;

    /* component quant table selector */
    jerr = _WRITE_BYTE(pFrame->comps[i].quant_sel,pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_SOF()\n");

  return jerr;
} /* EP_Write_SOF() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_SOF2
//
//  Purpose
//    Write a SOF2 (Start-Of-Frame) segment.
//
//  Context
//
//  Returns
//    a valid error code, or 0 if OK
//
//  Parameters
//    pFrame  The pointer to JPEG frame structure
//    pState  The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_SOF2(
  FRAME* pFrame,
  STATE* pState)
{
  int    i;
  int    len;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_SOF2()\n");

  len = 8 + (pFrame->ncomps * 3);

  if(pFrame->ncomps < 0 || pFrame->ncomps > 4)
  {
    jerr = IJL_UNSUPPORTED_FRAME;
    goto Exit;
  }

  jerr = _WRITE_WORD(0xff00 | MARKER_SOF2,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  if(pFrame->precision != 8)
  {
    jerr = IJL_UNSUPPORTED_FRAME;
    goto Exit;
  }

  /* sample precision */
  jerr = _WRITE_BYTE(pFrame->precision,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* JPEG image height */
  jerr = _WRITE_WORD(pFrame->height,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* JPEG image width */
  jerr = _WRITE_WORD(pFrame->width,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* number of channels */
  jerr = _WRITE_BYTE(pFrame->ncomps,pState);
  if(IJL_OK != jerr)
    goto Exit;

  for(i = 0; i < pFrame->ncomps; i++)
  {
    int sampling;

    /* component id */
    jerr = _WRITE_BYTE(pFrame->comps[i].ident,pState);
    if(IJL_OK != jerr)
      goto Exit;

    /* component sampling factors */
    sampling = (pFrame->comps[i].hsampling << 4) + pFrame->comps[i].vsampling;

    if(pFrame->ncomps == 1 && sampling != 0x11)
    {
      jerr = IJL_UNSUPPORTED_SUBSAMPLING;
      goto Exit;
    }

    jerr = _WRITE_BYTE(sampling,pState);
    if(IJL_OK != jerr)
      goto Exit;

    /* component quant table selector */
    jerr = _WRITE_BYTE(pFrame->comps[i].quant_sel,pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_SOF2()\n");

  return jerr;
} /* EP_Write_SOF2() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_DQT
//
//  Purpose
//    Writes a DQT (quantization table) segment including a quantization table.
//
//  Context
//
//  Returns
//    a valid error code, or 0 if OK
//
//  Parameters
//    precision  The quantization table precision
//                 0 - 8-bit tables,
//                 1 - 16 bit tables (non-baseline)
//    ident      Which table are we writing? The Frame maps these
//               identifiers onto the correct channels
//    quant      A single table of quantization values.
//    pState     The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_DQT(
  int    precision,
  int    ident,
  Ipp8u  quant[64],
  STATE* pState)
{
  int    i;
  int    len;
  int    prec_and_id;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_DQT()\n");

  if(precision != 0)
  {
    jerr = IJL_BAD_QUANT_TABLE;
    goto Exit;
  }

  len = 2 + 1 + DCTSIZE2;

  jerr = _WRITE_WORD(0xff00 | MARKER_DQT,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  prec_and_id = ((precision << 4) + ident);

  jerr = _WRITE_BYTE(prec_and_id,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* write quant table */
  for(i = 0; i < DCTSIZE2; i++)
  {
    jerr = _WRITE_BYTE(quant[i],pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_DQT()\n");

  return jerr;
} /* EP_Write_DQT() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_DHT_Ex
//
//  Purpose
//    Writes a DHT (huffman table) segment, including huffman tables.
//
//  Context
//
//  Returns
//    a valid error code, or 0 if ok.
//
//  Parameters
//    type     The type of table
//               0 - DC or lossless table
//               1 - AC table
//    ident    Huffman table destination identifier.
//             Specifies one of four possible destinations at the decoder
//             into which the Huffman table shall be installed.
//    pBits    The pointer to list of bits
//    pVals    The pointer to list of vals
//    pState   The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_DHT_Ex(
  int    type,
  int    ident,
  Ipp8u* pBits,
  Ipp8u* pVals,
  STATE* pState)
{
  int    i;
  int    len;
  int    nvals;
  int    type_and_id;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_DHT_Ex()\n");

  /* write marker */
  jerr = _WRITE_WORD(0xff00 | MARKER_DHT,pState);
  if(IJL_OK != jerr)
    goto Exit;

  for(i = 0, nvals = 0; i < 16; i++)
  {
    nvals += pBits[i];
  }

  /*
  // 2     - size for length field
  // 1     - size table class and table ident field
  // 16    - size list of bits
  // nvals - size list of vals
  */

  len = 2 + 1 + 16 + nvals;

  if(len > 324)
  {
    TRACE1(trERROR,"ERROR: len - %d\n",len);
    jerr = IJL_BAD_HUFFMAN_TABLE;
    goto Exit;
  }

  /* write length of segment */
  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  type_and_id = ((type << 4) + ident);

  jerr = _WRITE_BYTE(type_and_id,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* write list of bits */
  for(i = 0; i < 16; i++)
  {
    jerr = _WRITE_BYTE(pBits[i],pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

  /* write list of vals */
  for(i = 0; i < nvals; i++)
  {
    jerr = _WRITE_BYTE(pVals[i],pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_DHT_Ex()\n");

  return jerr;
} /* EP_Write_DHT_Ex() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_DHTs
//
//  Purpose
//    Writes a DHT (huffman table) segment, including huffman tables.
//    The maximum index is specified by jprops->maxhuffindex
//
//  Context
//
//  Returns
//    a valid error code, or 0 if ok.
//
//  Parameters
//    jprops  The pointer to jpeg properties structure.
//    pState  The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_DHTs(
  JPEG_PROPERTIES* jprops,
  STATE*           pState)
{
  int    i;
  int    j;
  int    len;
  int    nvals;
  int    type_and_id;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_DHTs()\n");

  for(nvals = i = 0; i < jprops->maxhuffindex * 2; i++)
  {
    for(j = 0; j < 16; j++)
    {
      nvals += jprops->rawhufftables[i].bits[j];
    }
  }

  len = 2 + 17 * jprops->maxhuffindex * 2 + nvals;

  jerr = _WRITE_WORD(0xff00 | MARKER_DHT,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  for(i = 0; i < jprops->maxhuffindex * 2; i++)
  {
    for(nvals = j = 0; j < 16; j++)
    {
      nvals += jprops->rawhufftables[i].bits[j];
    }

    len = 17 + nvals;
    if(len > 324)
    {
      TRACE1(trERROR,"ERROR: len - %d\n",len);
      jerr = IJL_BAD_HUFFMAN_TABLE;
      goto Exit;
    }

    type_and_id = ((jprops->rawhufftables[i].hclass << 4) + jprops->rawhufftables[i].ident);

    jerr = _WRITE_BYTE(type_and_id,pState);
    if(IJL_OK != jerr)
      goto Exit;

    for(j = 0; j < 16; j++)
    {
      jerr = _WRITE_BYTE(jprops->rawhufftables[i].bits[j],pState);
      if(IJL_OK != jerr)
        goto Exit;
    }

    for(j = 0; j < nvals; j++)
    {
      jerr = _WRITE_BYTE(jprops->rawhufftables[i].vals[j],pState);
      if(IJL_OK != jerr)
        goto Exit;
    }
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_DHTs()\n");

  return jerr;
} /* EP_Write_DHTs() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_SOS
//
//  Purpose
//    Writes a start-of-scan segment.
//
//  Context
//
//  Returns
//    a valid error code, or 0 if ok.
//
//  Parameters
//    DCHuff  Array of DC huffman table identifiers, where
//            component i uses dc table DCHuff[i]
//    ACHuff  Array of AC huffman table identifiers, where
//            component i uses ac table ACHuff[i]
//    pScan   The pointer to JPEG scan structure
//    pFrame  The pointer to JPEG frame structure
//    pState  The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_SOS(
  int    DCHuff[4],
  int    ACHuff[4],
  SCAN*  pScan,
  FRAME* pFrame,
  STATE* pState)
{
  int    i;
  int    len;
  int    huff_selector;
  int    approx_highlow;
  IJLERR jerr = IJL_OK;

  UNREFERENCED_PARAMETER(pFrame);

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_SOS()\n");

  len = 6 + (2 * pScan->ncomps);

  if(pScan->ncomps < 0 || pScan->ncomps > 4)
  {
    jerr = IJL_INVALID_JPEG_PROPERTIES;
    goto Exit;
  }

  jerr = _WRITE_WORD(0xff00 | MARKER_SOS,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE(pScan->ncomps,pState);
  if(IJL_OK != jerr)
    goto Exit;

  for(i = 0; i < pScan->ncomps; i++)
  {
    jerr = _WRITE_BYTE(pScan->comps[i].comp,pState);
    if(IJL_OK != jerr)
      goto Exit;

    huff_selector = ((DCHuff[pScan->comps[i].comp] << 4) + ACHuff[pScan->comps[i].comp]);

    jerr = _WRITE_BYTE(huff_selector,pState);
    if(IJL_OK != jerr)
      goto Exit;
  }

  /* Start of spectral selection */
  jerr = _WRITE_BYTE(pScan->start_spec,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* End of spectral selection */
  jerr = _WRITE_BYTE(pScan->end_spec,pState);
  if(IJL_OK != jerr)
    goto Exit;

  /* Successive approximation bit position (High-Low) */
  approx_highlow = ((pScan->approx_high << 4) + pScan->approx_low);

  jerr = _WRITE_BYTE(approx_highlow,pState);
  if(IJL_OK != jerr)
    goto Exit;

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_SOS()\n");

  return jerr;
} /* EP_Write_SOS() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_DRI
//
//  Purpose
//    Writes a restart interval specifier
//
//  Context
//
//  Returns
//    a valid error code, or 0 if ok.
//
//  Parameters
//    restart_interv  The granularity of the restart interval (in MCUs)
//    pState          The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_DRI(
  int    restart_interv,
  STATE* pState)
{
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_DRI()\n");

  jerr = _WRITE_WORD(0xff00 | MARKER_DRI,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(4,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(restart_interv,pState);
  if(IJL_OK != jerr)
    goto Exit;

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_DRI()\n");

  return jerr;
} /* EP_Write_DRI() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_RST
//
//  Purpose
//    Writes a restart interval marker
//
//  Context
//
//  Returns
//    a valid error code, or 0 if ok.
//
//  Parameters
//    restart_num  The number of the current restart interval
//    pState       The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_RST(
  int    restart_num,
  STATE* pState)
{
  IJLERR     jerr;
  IJL_MARKER marker_rst;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_RST()\n");

  switch(restart_num)
  {
  case 0:
    marker_rst = MARKER_RST0; break;
  case 1:
    marker_rst = MARKER_RST1; break;
  case 2:
    marker_rst = MARKER_RST2; break;
  case 3:
    marker_rst = MARKER_RST3; break;
  case 4:
    marker_rst = MARKER_RST4; break;
  case 5:
    marker_rst = MARKER_RST5; break;
  case 6:
    marker_rst = MARKER_RST6; break;
  case 7:
    marker_rst = MARKER_RST7; break;
  default:
    {
      TRACE1(trERROR,"ERROR: restart_num - %d\n",restart_num);
      jerr = IJL_BAD_RST_MARKER;
      goto Exit;
    }
  }

  jerr = _WRITE_WORD(0xff00 | marker_rst,pState);
  if(IJL_OK != jerr)
    goto Exit;

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_RST()\n");

  return jerr;
} /* EP_Write_RST() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EP_Write_APP14
//
//  Purpose
//    Write Adobe APP14 marker.
//  Context
//
//  Returns
//    A valid error code, or 0 if no error.
//
//  Parameters
//    version   - Version of Adobe App14 extension (100)
//    transform - Specifies Adobe transform
//                  1 - if the JPG color space is IJL_YCBCR,
//                  2 - if it's IJL_YCBCRA_FPX,
//                  0 - otherwise
//    pState    - The pointer to current IJL working state
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EP_Write_APP14(
  int    version,
  int    transform,
  int    flag0,
  int    flag1,
  STATE* pState)
{
  int    len;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in EP_Write_APP14()\n");

  jerr = _WRITE_WORD(0xff00 | MARKER_APP14,pState);
  if(IJL_OK != jerr)
    goto Exit;

  len = 2/*marker*/ + 5/*adobe*/ + 2/*version*/ + 2/*flags0*/ + 2/*flags1*/ + 1/*transform*/;

  jerr = _WRITE_WORD(len,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('A',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('d',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('o',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('b',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE('e',pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(version,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(flag0,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_WORD(flag1,pState);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _WRITE_BYTE(transform,pState);
  if(IJL_OK != jerr)
    goto Exit;

Exit:

  TRACE0(trCALL|trAFTER,"leave EP_Write_APP14()\n");

  return jerr;
} /* EP_Write_APP14() */


