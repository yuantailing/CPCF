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
#ifndef __DECODE_BUFFER_H__
#include "decode_buffer.h"
#endif



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DB_Init
//
//  Purpose
//    Initalizes decode buffer state variables to the zero-data state.
//
//  Context
//    Typically called once per external read to the IJL.
//    Some access modes (for example, the whole-image reads)
//    may access this function twice.
//
//  Parameters
//    pState      Pointer to the current IJL working state.
//
//  Returns
//    none
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DB_Init(
  STATE* pState)
{
  IppStatus status;

  if(NULL != pState->file)
  {
    pState->cur_entropy_ptr    = pState->JPGBuffer + JBUFSIZE;
    pState->end_entropy_ptr    = pState->JPGBuffer + JBUFSIZE;
    pState->entropy_bytes_left = 0;
  }

  pState->unread_marker     = 0;
  /* 1st component for multi-scan images. */
  pState->cur_scan_comp     = 0;

  status = IPPCALL(ippiDecodeHuffmanStateInit_JPEG_8u)(pState->u.pDecHuffState);
  if(ippStsNoErr != status)
  {
    return IJL_MEMORY_ERROR;
  }

  return IJL_OK;
} /* DB_Init() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    _READ_BYTE
//
//  Purpose
//    Read byte from the input JPEG stream.
//
//  Context
//
//  Returns
//    A valid error code, or 0 if OK.
//
//  Parameters
//    pState  Pointer to the current IJL state structure.
//    pByte   pointer to variable to store byte that has been read.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) _READ_BYTE(
  STATE* pState,
  int*   pByte)
{
  BOOL         bres;
  unsigned int count;

  if(pState->cur_entropy_ptr >= pState->end_entropy_ptr)
  {
    /* protect against buffer overrun */
    if(NULL == pState->file)
    {
      return IJL_ERR_DATA;
    }

    bres = ownReadFile(pState->file,pState->JPGBuffer,JBUFSIZE,&count);
    if(FALSE != bres && count == 0)
    {
      TRACE0(trERROR,"ERROR: ownReadFile\n");
      return IJL_FILE_ERROR;
    }

    pState->cur_entropy_ptr    = pState->start_entropy_ptr;
    pState->entropy_bytes_left = count;
  }

  pByte[0] = pState->cur_entropy_ptr[0];

  pState->cur_entropy_ptr++;
  pState->entropy_bytes_processed++;
  pState->entropy_bytes_left--;

  return IJL_OK;
} /* _READ_BYTE() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    _READ_WORD
//
//  Purpose
//    Read word (in Big-Endian) from the input JPEG stream.
//
//  Context
//
//  Returns
//    A valid error code, or 0 if OK.
//
//  Parameters
//    pState  Pointer to the current IJL state structure.
//    pWord   pointer to variable to store word that has been read.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) _READ_WORD(
  STATE* pState,
  int*   pWord)
{
  int    byte0;
  int    byte1;
  IJLERR jerr;

  jerr = _READ_BYTE(pState,&byte0);
  if(IJL_OK != jerr)
  {
    return jerr;
  }

  jerr = _READ_BYTE(pState,&byte1);
  if(IJL_OK != jerr)
  {
    return jerr;
  }

  pWord[0] = ( ((byte0 & 0xff) << 8) | (byte1 & 0xff) );

  return IJL_OK;
} /* _READ_WORD() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Buffer_Read_Bytes
//
//  Purpose:
//    Reads data from the input JPEG stream.
//
//  Context:
//
//  Parameters:
//    pState     Pointer to current IJL working state.
//
//  Returns:
//    A valid error code, or 0 if OK.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Buffer_Read_Bytes(
  STATE* pState)
{
  int i;
  int bres;
  unsigned int count;

  if(NULL == pState->file)
  {
    /* error if no more data bytes in buffer */
    if(pState->entropy_bytes_left == 0 && pState->unread_marker == 0)
    {
      pState->unread_marker = MARKER_EOI;
      return IJL_ERR_DATA;
    }
    return IJL_OK;
  }

  for(i = 0; i < pState->entropy_bytes_left; i++)
  {
    pState->start_entropy_ptr[i] = pState->cur_entropy_ptr[i];
  }

  bres = ownReadFile(
    pState->file,
    pState->JPGBuffer + pState->entropy_bytes_left,
    JBUFSIZE - pState->entropy_bytes_left,
    &count);

  if(FALSE != bres && count == 0)
  {
    if(pState->entropy_bytes_left == 0 && pState->unread_marker == 0)
    {
      pState->unread_marker = MARKER_EOI;
      return IJL_FILE_ERROR;
    }
  }

  pState->cur_entropy_ptr    = pState->JPGBuffer;
  pState->end_entropy_ptr    = pState->JPGBuffer +
                               pState->entropy_bytes_left +
                               count;
  pState->entropy_bytes_left = pState->entropy_bytes_left + count;

  return IJL_OK;
} /* Buffer_Read_Bytes() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Set_Decode_Fast_Path
//
//  Purpose:
//    1. Sets the cconversion_reqd and upsampling_reqd fields.
//    2. Sets max h and v sampling values.
//    3. Calculates number of MCUs.
//    4. Sets fast_processing for an optimized post-processing
//       path if appropriate.
//
//  Context:
//    Decoder calls here after a frame is detected.
//
//  Returns:
//    0        if no error.
//    Non-zero if an error was encountered.
//
//  Parameters:
//    *jprops     Pointer to current JPEG properties set.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Set_Decode_Fast_Path(
  JPEG_PROPERTIES* jprops)
{
  int i;
  IJL_CONTEXT* ctx = (IJL_CONTEXT*)jprops->state.ctx;

  /* Determine if CC is required, and set cconversion_reqd. */
  jprops->cconversion_reqd = TRUE;

  if((jprops->JPGColor == IJL_G)        || (jprops->JPGColor == IJL_RGB) ||
     (jprops->JPGColor == IJL_RGBA_FPX) || (jprops->JPGColor == IJL_OTHER) ||
     (jprops->DIBColor == IJL_G)        || (jprops->DIBColor == IJL_YCBCR))
  {
    jprops->cconversion_reqd = FALSE;
  }

  /*
  // Determine if US is required, and set upsampling_reqd.
  // Also determine the max h and v sampling values.
  */
  jprops->upsampling_reqd = FALSE;

  jprops->jframe.max_hsampling = 0;
  jprops->jframe.max_vsampling = 0;

  for(i = 0; i < jprops->jframe.ncomps; i++)
  {
    if(jprops->jframe.max_hsampling < jprops->jframe.comps[i].hsampling)
    {
      jprops->jframe.max_hsampling = jprops->jframe.comps[i].hsampling;
    }

    if(jprops->jframe.max_vsampling < jprops->jframe.comps[i].vsampling)
    {
      jprops->jframe.max_vsampling = jprops->jframe.comps[i].vsampling;
    }

    if((jprops->jframe.comps[i].hsampling != 1) || (jprops->jframe.comps[i].vsampling != 1))
    {
      if(jprops->DIBColor == IJL_YCBCR)
      {
        jprops->upsampling_reqd = FALSE;
      }
      else
      {
        jprops->upsampling_reqd = TRUE;
      }
    }
  }

  if(jprops->jinterleaveType == 1)
  {
    jprops->jframe.MCUheight = 8;
    jprops->jframe.MCUwidth  = 8;
  }
  else
  {
    jprops->jframe.MCUwidth  = 8 * jprops->jframe.max_hsampling;
    jprops->jframe.MCUheight = 8 * jprops->jframe.max_vsampling;
  }

  /* Determine the number of MCUs in horizontal and vertical directions. */
  jprops->numxMCUs = (jprops->JPGWidth + (8 * jprops->jframe.max_hsampling) - 1) /
    (8 * jprops->jframe.max_hsampling);
  jprops->numyMCUs = (jprops->JPGHeight + (8 * jprops->jframe.max_vsampling) - 1) /
    (8 * jprops->jframe.max_vsampling);

  jprops->jframe.horMCU   = jprops->numxMCUs;
  jprops->jframe.totalMCU = jprops->numxMCUs * jprops->numyMCUs;

  TRACE1(trINFO,"jprops->jframe.MCUheight = %d\n",jprops->jframe.MCUheight);
  TRACE1(trINFO,"jprops->jframe.MCUwidth  = %d\n",jprops->jframe.MCUwidth);
  TRACE1(trINFO,"jprops->numxMCUs = %d\n",jprops->numxMCUs);
  TRACE1(trINFO,"jprops->numyMCUs = %d\n",jprops->numyMCUs);

  /* Determine if/what fast decode path is supported/appropriate. */
  jprops->fast_processing = IJL_NO_CC_OR_US;

  if( (jprops->jframe.ncomps == 3) &&
      (jprops->JPGColor == IJL_YCBCR) && (jprops->DIBColor == IJL_YCBCR) &&
     ((jprops->jframe.comps[0].hsampling != 2) || (jprops->jframe.comps[0].vsampling != 1) ||
      (jprops->jframe.comps[1].hsampling != 1) || (jprops->jframe.comps[1].vsampling != 1) ||
      (jprops->jframe.comps[2].hsampling != 1) || (jprops->jframe.comps[2].vsampling != 1)) )
  {
    /* only YCbCr 422 -> YCbYCr decoding is supported */
    return IJL_INVALID_JPEG_PROPERTIES;
  }

  if((jprops->jframe.ncomps < 1) || (jprops->jframe.ncomps > 255))
  {
    return IJL_ERR_COMP;
  }

  if(!(jprops->jinterleaveType) || (jprops->progressive_found))
  {
    if((jprops->upsampling_reqd) && (jprops->cconversion_reqd))
    {
      if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 3)
      {
        /* 3 channel JPEG fast paths supported: */

        if(jprops->DIBColor == IJL_RGB)
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_411_YCBCR_111_RGB;
              ctx->__g_us_and_cc_mcu = YCbCr_411_to_RGB_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_422_YCBCR_111_RGB;
              ctx->__g_us_and_cc_mcu = YCbCr_422_to_RGB_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else
          {
            ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
          }
        }
        else if(jprops->DIBColor == IJL_BGR)
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_411_YCBCR_111_BGR;
              ctx->__g_us_and_cc_mcu = YCbCr_411_to_BGR_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_422_YCBCR_111_BGR;
              ctx->__g_us_and_cc_mcu = YCbCr_422_to_BGR_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else
          {
            ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
          }
        }
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 4)
      {
        if(jprops->DIBColor == IJL_RGBA_FPX)
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_411_YCBCR_1111_RGBA_FPX;
              ctx->__g_us_and_cc_mcu = YCbCr_411_to_RGBA_FPX_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_422_YCBCR_1111_RGBA_FPX;
              ctx->__g_us_and_cc_mcu = YCbCr_422_to_RGBA_FPX_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else
          {
            ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
          }
        }
      }
      else if(jprops->jframe.ncomps == 4 && jprops->DIBChannels == 4)
      {
        /* 4 channel JPEG fast paths supported: */

        if(jprops->DIBColor == IJL_RGBA_FPX)
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1) &&
             (jprops->jframe.comps[3].hsampling == 2) && (jprops->jframe.comps[3].vsampling == 1))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_4224_YCBCRA_FPX_1111_RGBA_FPX;
              ctx->__g_us_and_cc_mcu = YCbCrA_FPX_4224_to_RGBA_FPX_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1) &&
             (jprops->jframe.comps[3].hsampling == 2) && (jprops->jframe.comps[3].vsampling == 2))
          {
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              jprops->fast_processing = IJL_4114_YCBCRA_FPX_1111_RGBA_FPX;
              ctx->__g_us_and_cc_mcu = YCbCrA_FPX_4114_to_RGBA_FPX_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
            }
          }
          else
          {
            ctx->__g_us_and_cc_mcu = US_CC_General_YCbCr_MCU;
          }
        }
      }
      else /* Number of components is from 1-255 but not either 3 or 4. */
      {
        /* No fast paths supported. */
      }
    }
    else if((jprops->upsampling_reqd) && !(jprops->cconversion_reqd))
    {
      if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 1))
      {
        if(jprops->progressive_found == 0)
        {
          ctx->__g_us_and_cc_mcu = US_General_MCU;
        }
        else
        {
          ctx->__g_us_and_cc_mcu = US_General_P_MCU;
        }
      }
      else if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 3))
      {
        if(jprops->DIBColor == IJL_BGR)
        {
          if(jprops->progressive_found == 0)
          {
            ctx->__g_us_and_cc_mcu = US_General_MCU;
          }
          else
          {
            ctx->__g_us_and_cc_mcu = US_General_P_MCU;
          }
        }
        else
        {
          /* upsampling 411 supported. */
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_411_OTHER_111_OTHER;
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              ctx->__g_us_and_cc_mcu = OTHER_411_to_OTHER_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              if(jprops->progressive_found == 0)
              {
                ctx->__g_us_and_cc_mcu = US_General_MCU;
              }
              else
              {
                ctx->__g_us_and_cc_mcu = US_General_P_MCU;
              }
            }
          }
          /* upsampling 422 supported. */
          else if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
                  (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
                  (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_422_OTHER_111_OTHER;
            if(jprops->upsampling_type == IJL_BOX_FILTER)
            {
              ctx->__g_us_and_cc_mcu = OTHER_422_to_OTHER_MCU;
            }
            else if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
            {
              if(jprops->progressive_found == 0)
              {
                ctx->__g_us_and_cc_mcu = US_General_MCU;
              }
              else
              {
                ctx->__g_us_and_cc_mcu = US_General_P_MCU;
              }
            }
          }
          else
          {
            if(jprops->progressive_found == 0)
            {
              ctx->__g_us_and_cc_mcu = US_General_MCU;
            }
            else
            {
              ctx->__g_us_and_cc_mcu = US_General_P_MCU;
            }
          }
        } /* DIBColor != IJL_BGR */
      }
      else if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 4))
      {
        if(jprops->progressive_found == 0)
        {
          ctx->__g_us_and_cc_mcu = US_General_MCU;
        }
        else
        {
          ctx->__g_us_and_cc_mcu = US_General_P_MCU;
        }
      }
      else if((jprops->jframe.ncomps == 4) && (jprops->DIBChannels == 3))
      {
        if(jprops->progressive_found == 0)
        {
          ctx->__g_us_and_cc_mcu = US_General_MCU;
        }
        else
        {
          ctx->__g_us_and_cc_mcu = US_General_P_MCU;
        }
      }
      else if((jprops->jframe.ncomps == 4) && (jprops->DIBChannels == 4))
      {
        if(jprops->progressive_found == 0)
        {
          ctx->__g_us_and_cc_mcu = US_General_MCU;
        }
        else
        {
          ctx->__g_us_and_cc_mcu = US_General_P_MCU;
        }
      }
    }
    else if(!(jprops->upsampling_reqd) && (jprops->cconversion_reqd))
    {
      if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 3))
      {
        /* Fast paths supported: */
        if(jprops->DIBColor == IJL_RGB)
        {
          jprops->fast_processing = IJL_111_YCBCR_111_RGB;
          ctx->__g_us_and_cc_mcu = YCbCr_111_to_RGB_MCU;
        }
        else if(jprops->DIBColor == IJL_BGR)
        {
          jprops->fast_processing = IJL_111_YCBCR_111_BGR;
          ctx->__g_us_and_cc_mcu = YCbCr_111_to_BGR_MCU;
        }
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 4)
      {
        if(jprops->DIBColor == IJL_RGBA_FPX)
        {
          jprops->fast_processing = IJL_111_YCBCR_1111_RGBA_FPX;
          ctx->__g_us_and_cc_mcu = YCbCr_111_to_RGBA_FPX_MCU;
        }
      }
      else if(jprops->jframe.ncomps == 4 && jprops->DIBChannels == 4)
      {
        if(jprops->DIBColor == IJL_RGBA_FPX)
        {
           jprops->fast_processing = IJL_1111_YCBCRA_FPX_1111_RGBA_FPX;
           ctx->__g_us_and_cc_mcu = YCbCrA_FPX_1111_to_RGBA_FPX_MCU;
        }
      }
    }
    else if((!jprops->upsampling_reqd) && (!jprops->cconversion_reqd))
    {
      if(jprops->jframe.ncomps == 1 && jprops->DIBChannels == 1)
      {
        ctx->__g_us_and_cc_mcu = Y_111_to_Y_MCU;
      }
      if(jprops->jframe.ncomps == 1 && jprops->DIBChannels != 1)
      {
        ctx->__g_us_and_cc_mcu = Output_Interleave_General_MCU;
      }
      else if(jprops->jframe.ncomps == 2)
      {
        ctx->__g_us_and_cc_mcu = Output_Interleave_General_MCU;
      }
      else if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 1))
      {
        ctx->__g_us_and_cc_mcu = Output_Interleave_General_MCU;
      }
      else if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 2))
      {
        if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
           (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
           (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
        {
          jprops->fast_processing = IJL_YCBCR_YCBYCR;
          ctx->__g_us_and_cc_mcu = YCbCr_422_to_YCbYCr_MCU;
        }
      }
      else if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 3))
      {
        if(jprops->DIBColor == IJL_BGR)
        {
          jprops->fast_processing = IJL_NO_CC_OR_US;
          ctx->__g_us_and_cc_mcu = Output_Interleave_General_MCU;
        }
        else
        {
          jprops->fast_processing = IJL_111_OTHER_111_OTHER;
          ctx->__g_us_and_cc_mcu = OTHER_111_to_OTHER_MCU;
        }
      }
      else if((jprops->jframe.ncomps == 3) && (jprops->DIBChannels == 4))
      {
        if(jprops->DIBColor == IJL_RGBA_FPX)
        {
          jprops->fast_processing = IJL_111_RGB_1111_RGBA_FPX;
          ctx->__g_us_and_cc_mcu = OTHER_111_to_OTHER4_MCU;
        }
        else
        {
          ctx->__g_us_and_cc_mcu = Output_Interleave_General_MCU;
        }
      }
      else if((jprops->jframe.ncomps == 4) && (jprops->DIBChannels == 3))
      {
        jprops->fast_processing = IJL_NO_CC_OR_US;
        ctx->__g_us_and_cc_mcu = Output_Interleave_General_MCU;
      }
      else if((jprops->jframe.ncomps == 4) && (jprops->DIBChannels == 4))
      {
        jprops->fast_processing = IJL_1111_RGBA_FPX_1111_RGBA_FPX;
        ctx->__g_us_and_cc_mcu = OTHER_1111_to_OTHER_MCU;
      }
    }
  }
  else
  {
    if(jprops->DIBColor == IJL_YCBCR)
    {
      ctx->__g_us_and_cc_mcu = YCbCr_422_to_YCbYCr_NI_MCU;
    }
    else
    {
      if(FALSE == jprops->progressive_found)
      {
        ctx->__g_us_and_cc_mcu = US_General_MCU;
      }
      else
      {
        ctx->__g_us_and_cc_mcu = US_General_P_MCU;
      }
    }
  }

  return IJL_OK;
} /* Set_Decode_Fast_Path() */

