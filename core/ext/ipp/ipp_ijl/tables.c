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
#ifndef __TABLES_H__
#include "tables/tables.h"
#endif





/* ///////////////////////////////////////////////////////////////////////////
// Static Tables
*/

/* Zigzag of Default Luminance Quantization Table */
LOCVAR(const Ipp8u) DefaultLumQuant[] =
{
  ( 16), ( 11), ( 12), ( 14), ( 12), ( 10), ( 16), ( 14),
  ( 13), ( 14), ( 18), ( 17), ( 16), ( 19), ( 24), ( 40),
  ( 26), ( 24), ( 22), ( 22), ( 24), ( 49), ( 35), ( 37),
  ( 29), ( 40), ( 58), ( 51), ( 61), ( 60), ( 57), ( 51),
  ( 56), ( 55), ( 64), ( 72), ( 92), ( 78), ( 64), ( 68),
  ( 87), ( 69), ( 55), ( 56), ( 80), (109), ( 81), ( 87),
  ( 95), ( 98), (103), (104), (103), ( 62), ( 77), (113),
  (121), (112), (100), (120), ( 92), (101), (103), ( 99)
};


/* Zigzag of Default Chrominance Quantization Table */
LOCVAR(const Ipp8u) DefaultChromQuant[] =
{
  (17), ( 18), ( 18), ( 24), ( 21), ( 24), ( 47),  (26),
  (26), ( 47), ( 99), ( 66), ( 56), ( 66), ( 99),  (99),
  (99), ( 99), ( 99), ( 99), ( 99), ( 99), ( 99),  (99),
  (99), ( 99), ( 99), ( 99), ( 99), ( 99), ( 99),  (99),
  (99), ( 99), ( 99), ( 99), ( 99), ( 99), ( 99),  (99),
  (99), ( 99), ( 99), ( 99), ( 99), ( 99), ( 99),  (99),
  (99), ( 99), ( 99), ( 99), ( 99), ( 99), ( 99),  (99),
  (99), ( 99), ( 99), ( 99), ( 99), ( 99), ( 99),  (99)
};


LOCVAR(const Ipp8u) DefaultLuminanceDCBits[] =
{
  0x000, 0x001, 0x005, 0x001, 0x001, 0x001, 0x001, 0x001,
  0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};


LOCVAR(const Ipp8u) DefaultLuminanceDCValues[] =
{
  0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
  0x008, 0x009, 0x00a, 0x00b
};


LOCVAR(const Ipp8u) DefaultChrominanceDCBits[] =
{
  0x000, 0x003, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
  0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000
};


LOCVAR(const Ipp8u) DefaultChrominanceDCValues[] =
{
  0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
  0x008, 0x009, 0x00a, 0x00b
};


LOCVAR(const Ipp8u) DefaultLuminanceACBits[] =
{
  0x000, 0x002, 0x001, 0x003, 0x003, 0x002, 0x004, 0x003,
  0x005, 0x005, 0x004, 0x004, 0x000, 0x000, 0x001, 0x07d
};


LOCVAR(const Ipp8u) DefaultLuminanceACValues[] =
{
  0x001, 0x002, 0x003, 0x000, 0x004, 0x011, 0x005, 0x012,
  0x021, 0x031, 0x041, 0x006, 0x013, 0x051, 0x061, 0x007,
  0x022, 0x071, 0x014, 0x032, 0x081, 0x091, 0x0a1, 0x008,
  0x023, 0x042, 0x0b1, 0x0c1, 0x015, 0x052, 0x0d1, 0x0f0,
  0x024, 0x033, 0x062, 0x072, 0x082, 0x009, 0x00a, 0x016,
  0x017, 0x018, 0x019, 0x01a, 0x025, 0x026, 0x027, 0x028,
  0x029, 0x02a, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039,
  0x03a, 0x043, 0x044, 0x045, 0x046, 0x047, 0x048, 0x049,
  0x04a, 0x053, 0x054, 0x055, 0x056, 0x057, 0x058, 0x059,
  0x05a, 0x063, 0x064, 0x065, 0x066, 0x067, 0x068, 0x069,
  0x06a, 0x073, 0x074, 0x075, 0x076, 0x077, 0x078, 0x079,
  0x07a, 0x083, 0x084, 0x085, 0x086, 0x087, 0x088, 0x089,
  0x08a, 0x092, 0x093, 0x094, 0x095, 0x096, 0x097, 0x098,
  0x099, 0x09a, 0x0a2, 0x0a3, 0x0a4, 0x0a5, 0x0a6, 0x0a7,
  0x0a8, 0x0a9, 0x0aa, 0x0b2, 0x0b3, 0x0b4, 0x0b5, 0x0b6,
  0x0b7, 0x0b8, 0x0b9, 0x0ba, 0x0c2, 0x0c3, 0x0c4, 0x0c5,
  0x0c6, 0x0c7, 0x0c8, 0x0c9, 0x0ca, 0x0d2, 0x0d3, 0x0d4,
  0x0d5, 0x0d6, 0x0d7, 0x0d8, 0x0d9, 0x0da, 0x0e1, 0x0e2,
  0x0e3, 0x0e4, 0x0e5, 0x0e6, 0x0e7, 0x0e8, 0x0e9, 0x0ea,
  0x0f1, 0x0f2, 0x0f3, 0x0f4, 0x0f5, 0x0f6, 0x0f7, 0x0f8,
  0x0f9, 0x0fa
};


LOCVAR(const Ipp8u) DefaultChrominanceACBits[] =
{
  0x000, 0x002, 0x001, 0x002, 0x004, 0x004, 0x003, 0x004,
  0x007, 0x005, 0x004, 0x004, 0x000, 0x001, 0x002, 0x077
};


LOCVAR(const Ipp8u) DefaultChrominanceACValues[] =
{
  0x000, 0x001, 0x002, 0x003, 0x011, 0x004, 0x005, 0x021,
  0x031, 0x006, 0x012, 0x041, 0x051, 0x007, 0x061, 0x071,
  0x013, 0x022, 0x032, 0x081, 0x008, 0x014, 0x042, 0x091,
  0x0a1, 0x0b1, 0x0c1, 0x009, 0x023, 0x033, 0x052, 0x0f0,
  0x015, 0x062, 0x072, 0x0d1, 0x00a, 0x016, 0x024, 0x034,
  0x0e1, 0x025, 0x0f1, 0x017, 0x018, 0x019, 0x01a, 0x026,
  0x027, 0x028, 0x029, 0x02a, 0x035, 0x036, 0x037, 0x038,
  0x039, 0x03a, 0x043, 0x044, 0x045, 0x046, 0x047, 0x048,
  0x049, 0x04a, 0x053, 0x054, 0x055, 0x056, 0x057, 0x058,
  0x059, 0x05a, 0x063, 0x064, 0x065, 0x066, 0x067, 0x068,
  0x069, 0x06a, 0x073, 0x074, 0x075, 0x076, 0x077, 0x078,
  0x079, 0x07a, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087,
  0x088, 0x089, 0x08a, 0x092, 0x093, 0x094, 0x095, 0x096,
  0x097, 0x098, 0x099, 0x09a, 0x0a2, 0x0a3, 0x0a4, 0x0a5,
  0x0a6, 0x0a7, 0x0a8, 0x0a9, 0x0aa, 0x0b2, 0x0b3, 0x0b4,
  0x0b5, 0x0b6, 0x0b7, 0x0b8, 0x0b9, 0x0ba, 0x0c2, 0x0c3,
  0x0c4, 0x0c5, 0x0c6, 0x0c7, 0x0c8, 0x0c9, 0x0ca, 0x0d2,
  0x0d3, 0x0d4, 0x0d5, 0x0d6, 0x0d7, 0x0d8, 0x0d9, 0x0da,
  0x0e2, 0x0e3, 0x0e4, 0x0e5, 0x0e6, 0x0e7, 0x0e8, 0x0e9,
  0x0ea, 0x0f2, 0x0f3, 0x0f4, 0x0f5, 0x0f6, 0x0f7, 0x0f8,
  0x0f9, 0x0fa
};




/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    SetDefaultTables
//
//  Purpose
//    Called during interface initalization to use a default set of
//    JPEG huffman and quantization tables
//
//  Context
//
//  Returns
//    IJL_OK or error code
//
//  Parameters
//    jprops  pointer to JPEG_PROPERTIES
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) SetDefaultTables(
  JPEG_PROPERTIES* jprops)
{
  int i;
  int have_user_quant_tables = FALSE;
  int have_user_huff_tables = FALSE;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in SetDefaultTables()\n");

  /* check for user quant tables */
  for(i = 0; i < 4; i++)
  {
    if(NULL != jprops->rawquanttables[i].quantizer)
    {
      have_user_quant_tables = jprops->use_external_qtables;
      if(have_user_quant_tables)
      {
        TRACE2(trINFO,"found user quant table num - %d, addr - 0x%08X\n",
          i,jprops->rawquanttables[i].quantizer);
      }
    }
  }

  /* check for user huffman tables */
  for(i = 0; i < 8; i++)
  {
    if(NULL != jprops->rawhufftables[i].bits &&
       NULL != jprops->rawhufftables[i].vals)
    {
      have_user_huff_tables = jprops->use_external_htables;
      if(have_user_huff_tables)
      {
        TRACE2(trINFO,"found user huff table [BITS] num - %d, addr - 0x%08X\n",
          i,jprops->rawhufftables[i].bits);
        TRACE2(trINFO,"found user huff table [VALS] num - %d, addr - 0x%08X\n",
          i,jprops->rawhufftables[i].vals);
      }
    }
  }

  if(FALSE == have_user_quant_tables)
  {
    TRACE0(trINFO,"set default quant tables\n");
    /* Set default quantization tables */
    jprops->maxquantindex = 2;
    jprops->nqtables      = 2;

    jprops->rawquanttables[0].quantizer = (Ipp8u*)&DefaultLumQuant[0];
    jprops->rawquanttables[0].ident     = 0;

    jprops->rawquanttables[1].quantizer = (Ipp8u*)&DefaultChromQuant[0];
    jprops->rawquanttables[1].ident     = 1;

    jprops->rawquanttables[2].quantizer = NULL;
    jprops->rawquanttables[2].ident     = 0;

    jprops->rawquanttables[3].quantizer = NULL;
    jprops->rawquanttables[3].ident     = 0;

  } /* default quant tables */

  if(FALSE == have_user_huff_tables)
  {
    TRACE0(trINFO,"set default huffman tables\n");
    /* Set default huffman tables */
    jprops->nhuffActables = jprops->JPGChannels == 1 ? 1 : 2;
    jprops->nhuffDctables = jprops->JPGChannels == 1 ? 1 : 2;
    jprops->maxhuffindex  = jprops->JPGChannels == 1 ? 1 : 2;

    jprops->rawhufftables[0].bits   = (Ipp8u*)&DefaultLuminanceDCBits[0];
    jprops->rawhufftables[0].vals   = (Ipp8u*)&DefaultLuminanceDCValues[0];
    jprops->rawhufftables[0].hclass = 0;
    jprops->rawhufftables[0].ident  = 0;

    jprops->rawhufftables[1].bits   = (Ipp8u*)&DefaultLuminanceACBits[0];
    jprops->rawhufftables[1].vals   = (Ipp8u*)&DefaultLuminanceACValues[0];
    jprops->rawhufftables[1].hclass = 1;
    jprops->rawhufftables[1].ident  = 0;

    jprops->rawhufftables[2].bits   = (Ipp8u*)&DefaultChrominanceDCBits[0];
    jprops->rawhufftables[2].vals   = (Ipp8u*)&DefaultChrominanceDCValues[0];
    jprops->rawhufftables[2].hclass = 0;
    jprops->rawhufftables[2].ident  = 1;

    jprops->rawhufftables[3].bits   = (Ipp8u*)&DefaultChrominanceACBits[0];
    jprops->rawhufftables[3].vals   = (Ipp8u*)&DefaultChrominanceACValues[0];
    jprops->rawhufftables[3].hclass = 1;
    jprops->rawhufftables[3].ident  = 1;

    jprops->rawhufftables[4].bits   = NULL;
    jprops->rawhufftables[4].vals   = NULL;
    jprops->rawhufftables[4].hclass = 0;
    jprops->rawhufftables[4].ident  = 0;

    jprops->rawhufftables[5].bits   = NULL;
    jprops->rawhufftables[5].vals   = NULL;
    jprops->rawhufftables[5].hclass = 0;
    jprops->rawhufftables[5].ident  = 0;

    jprops->rawhufftables[6].bits   = NULL;
    jprops->rawhufftables[6].vals   = NULL;
    jprops->rawhufftables[6].hclass = 0;
    jprops->rawhufftables[6].ident  = 0;

    jprops->rawhufftables[7].bits   = NULL;
    jprops->rawhufftables[7].vals   = NULL;
    jprops->rawhufftables[7].hclass = 0;
    jprops->rawhufftables[7].ident  = 0;

    jprops->HuffIdentifierDC[0]     = 0;
    jprops->HuffIdentifierDC[1]     = 1;
    jprops->HuffIdentifierDC[2]     = 1;
    jprops->HuffIdentifierDC[3]     = 1;

    jprops->HuffIdentifierAC[0]     = 0;
    jprops->HuffIdentifierAC[1]     = 1;
    jprops->HuffIdentifierAC[2]     = 1;
    jprops->HuffIdentifierAC[3]     = 1;
  } /* default huffman tables */

  TRACE0(trCALL|trAFTER,"leave SetDefaultTables()\n");

  return jerr;
} /* SetDefaultTables() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Scale_Char_Matrix
//
//  Purpose
//    This function modifies the input quantization tables by scaling with
//    a quality equation defined by the independent JPEG group
//    (see note below).
//
//  Context
//    Used only by the encoder to format the quantization tables
//    for use in the quantizer.
//
//  Returns
//    IJL_OK or error code
//
//  Parameters
//    pSrcTable   input JPEG-format quantization table
//    nQuality    quality level (1-100),
//                1 = awful, 100 = best quality
//    pDstTable   16-bit precision quantization table
//                scaled for quality.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Scale_Char_Matrix(
  Ipp8u* pSrcTable,
  int    nQuality,
  Ipp8u* pDstTable)
{
  Ipp8u rawquant[DCTSIZE2];

  memcpy(rawquant,pSrcTable,DCTSIZE2);

  IPPCALL(ippiQuantFwdRawTableInit_JPEG_8u)(rawquant,nQuality);
  IPPCALL(ippsCopy_8u)(rawquant,pDstTable,DCTSIZE2);

  return IJL_OK;
} /* Scale_Char_Matrix() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    BuildEncoderHuffmanTable
//
//  Purpose
//    Builds a JPEG compliant huffman table given the raw bits and huffval
//    table entries. The table consists of sorted huffval and huffelem
//    sub-tables.
//
//  Context
//    Called only by the encoder
//
//  Returns
//    IJL_OK or error code
//
//  Parameters
//    pListBits   A list of bit-length frequencies
//    pListVals   A list of input huffman values
//    pEncHuffTbl A pointer to the output huffman table
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) BuildEncoderHuffmanTable(
  const Ipp8u*   pListBits,
  const Ipp8u*   pListVals,
  HUFFMAN_TABLE* pEncHuffTbl)
{
  IppStatus status;

  status = IPPCALL(ippiEncodeHuffmanSpecInit_JPEG_8u)(
    pListBits,
    pListVals,
    pEncHuffTbl->u.pEncHuffTbl);

  if(ippStsNoErr != status)
  {
    return IJL_BAD_HUFFMAN_TABLE;
  }

  return IJL_OK;
} /* BuildEncoderHuffmanTable() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    BuildDecoderHuffmanTable
//
//  Purpose
//    Re-format the data in the "raw" huffman table format (the format used
//    in the JPEG datastream) into a format optimized for our huffman decoder.
//    This involves building a table of huffval, huffelem, valptr, mincode,
//    and maxcode that are used to do a table-lookup of 8-entropy bits
//    at a time to (hopefully) determine both the run length and bit count
//    of the data symbol in a single read. If this fails, the mincode and
//    maxcode entries are used to get data the slow way. See the JPEG format
//    specification for hints on this practice.
//
//  Context
//
//  Returns
//    valid error code, or 0 for OK
//
//  Parameters
//    pListBits    bit-length frequency
//                 from the raw huffman tables
//    pListVlas    Huffman values - see the JPEG specification
//    pDecHuffTbl  A pointer to the "result" huffman table
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) BuildDecoderHuffmanTable(
  const Ipp8u*   pListBits,
  const Ipp8u*   pListVals,
  HUFFMAN_TABLE* pDecHuffTbl)
{
  IppStatus status;

  status = IPPCALL(ippiDecodeHuffmanSpecInit_JPEG_8u)(
    pListBits,
    pListVals,
    pDecHuffTbl->u.pDecHuffTbl);

  if(ippStsNoErr != status)
  {
    return IJL_BAD_HUFFMAN_TABLE;
  }

  return IJL_OK;
} /* BuildDecoderHuffmanTable() */

