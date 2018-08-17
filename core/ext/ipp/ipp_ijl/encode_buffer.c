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
#ifndef __ENCODE_BUFFER_H__
#include "encode_buffer.h"
#endif


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EB_Init
//
//  Purpose
//    Initalize the encoder parameters.
//    The Encoder is initalized to zero data.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//    pBuf      Pointer to the output buffer.
//    nBufSize  Maximum number of bytes in the output buffer.
//    pState    Pointer to current IJL state structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EB_Init(
  Ipp8u* pBuf,
  int    nBufSize,
  STATE* pState)
{
  IJLERR    jerr = IJL_OK;
  IppStatus status;

  TRACE0(trCALL|trBEFORE,"enter in EB_Init()\n");

  if(NULL != pState->file)
  {
    TRACE0(trINFO,"work with file\n");
    pState->entropy_buf_maxsize     = JBUFSIZE;
    pState->start_entropy_ptr       = (Ipp8u*)&pState->JPGBuffer;
    pState->end_entropy_ptr         = pState->start_entropy_ptr +
                                      pState->entropy_buf_maxsize;
    pState->cur_entropy_ptr         = pState->start_entropy_ptr;
    pState->entropy_bytes_left      = pState->entropy_buf_maxsize;
    pState->entropy_bytes_processed = 0;
  }
  else
  {
    TRACE0(trINFO,"work with buffer\n");
    pState->entropy_buf_maxsize     = nBufSize;
    pState->start_entropy_ptr       = pBuf;
    pState->end_entropy_ptr         = pState->start_entropy_ptr +
                                      pState->entropy_buf_maxsize;
    pState->cur_entropy_ptr         = pState->start_entropy_ptr;
    pState->entropy_bytes_left      = pState->entropy_buf_maxsize;
    pState->entropy_bytes_processed = 0;
  }

  status = IPPCALL(ippiEncodeHuffmanStateInit_JPEG_8u)(pState->u.pEncHuffState);
  if(ippStsNoErr != status)
  {
    jerr = IJL_MEMORY_ERROR;
    goto Exit;
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave EB_Init()\n");

  return jerr;
} /* EB_Init() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    _WRITE_BYTE
//
//  Purpose
//    Writes byte to the output JPEG stream.
//
//  Context
//
//  Returns
//    A valid error code, or 0 if OK.
//
//  Parameters
//    Data    input data to write.
//    pState  Pointer to the current IJL state structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) _WRITE_BYTE(
  int    Byte,
  STATE* pState)
{
  BOOL bres;
  unsigned int count;

  if(pState->cur_entropy_ptr >= pState->end_entropy_ptr)
  {
    /* protect against buffer overrun */
    if(NULL == pState->file)
    {
      return IJL_BUFFER_TOO_SMALL;
    }

    bres = ownWriteFile(pState->file,pState->JPGBuffer,JBUFSIZE,&count);

    if(FALSE == bres || JBUFSIZE != count)
    {
      return IJL_FILE_ERROR;
    }

    pState->cur_entropy_ptr = pState->start_entropy_ptr;
  }

  pState->cur_entropy_ptr[0] = (Ipp8u)Byte;

  pState->cur_entropy_ptr++;
  pState->entropy_bytes_processed++;
  pState->entropy_bytes_left--;

  return IJL_OK;
} /* _WRITE_BYTE() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    _WRITE_WORD
//
//  Purpose
//    Writes word (in Big-Endian) to the output JPEG stream.
//
//  Context
//
//  Returns
//    A valid error code, or 0 if OK.
//
//  Parameters
//    Data    input data to write.
//    pState  Pointer to the current IJL state structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) _WRITE_WORD(
  int    Word,
  STATE* pState)
{
  IJLERR jerr;

  jerr = _WRITE_BYTE(((Word >> 8) & 0xff),pState);
  if(IJL_OK != jerr)
  {
    return jerr;
  }

  jerr = _WRITE_BYTE((Word & 0xff),pState);
  if(IJL_OK != jerr)
  {
    return jerr;
  }

  return IJL_OK;
} /* _WRITE_WORD() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Flush_Buffer_To_File
//
//  Purpose
//    At completion of a session of writing to the
//    output JPEG stream, data may still reside in
//    temporary buffers. This routine flushes all
//    partially-full buffers to the output stream.
//
//  Context
//
//  Returns
//    A valid error code, or 0 if OK
//
//  Parameters
//    pState  Pointer to current IJL state structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Flush_Buffer_To_File(
  STATE* pState)
{
  BOOL   bres;
  unsigned int  count;
  unsigned int  bytes_to_write;
  IJLERR jerr = IJL_OK;

  /* Write the remaining data to the file. */
  if(NULL != pState->file)
  {
    bytes_to_write = pState->cur_entropy_ptr - pState->start_entropy_ptr;

    bres = ownWriteFile(pState->file,pState->JPGBuffer,bytes_to_write,&count);

    if(FALSE == bres || bytes_to_write != count)
    {
      jerr = IJL_FILE_ERROR;
      goto Exit;
    }

    pState->cur_entropy_ptr = pState->start_entropy_ptr;
    pState->entropy_bytes_left = pState->entropy_buf_maxsize;
  }

Exit:

  return jerr;
} /* Flush_Buffer_To_File() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Set_Encode_Fast_Path
//
//  Purpose
//    Sets fast_processing for an optimized
//    pre-processing path if appropriate.
//
//  Context
//    Encoder calls here before encoding a frame.
//
//  Returns
//    IJL_OK   if no error.
//    Non-zero if an error was encountered.
//
//  Parameters
//    jprops  Pointer to current JPEG properties set.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Set_Encode_Fast_Path(
  JPEG_PROPERTIES* jprops)
{
  IJL_CONTEXT* ctx = (IJL_CONTEXT*)jprops->state.ctx;
  IJLERR jerr = IJL_OK;

  /* Determine if/what fast path is supported. */
  jprops->fast_processing = IJL_NO_CC_OR_US;

  if(!jprops->jinterleaveType)
  {
    if((jprops->upsampling_reqd) && (jprops->cconversion_reqd))
    {
      if((jprops->jframe.ncomps < 1) || (jprops->jframe.ncomps > 255))
      {
        jerr = IJL_ERR_COMP;
        goto Exit;
      }
      else if(jprops->jframe.ncomps == 3)
      {
        /* 3 channel JPEG fast paths supported: */

        if((jprops->DIBColor == IJL_RGB) && (jprops->DIBChannels == 3))
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_411_YCBCR_111_RGB;
            ctx->__g_cc_and_ss_mcu = RGB_to_YCbCr_411_MCU;
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_422_YCBCR_111_RGB;
            ctx->__g_cc_and_ss_mcu = RGB_to_YCbCr_422_MCU;
          }
          else
          {
          }
        }
        else if((jprops->DIBColor == IJL_RGB) && (jprops->DIBChannels == 4))
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_NO_CC_OR_US;
            ctx->__g_cc_and_ss_mcu = CC_SS_RGB_to_YCbCr_General_MCU;
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_NO_CC_OR_US;
            ctx->__g_cc_and_ss_mcu = CC_SS_RGB_to_YCbCr_General_MCU;
          }
          else
          {
          }
        }
        else if((jprops->DIBColor == IJL_BGR) && (jprops->DIBChannels == 3))
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_411_YCBCR_111_BGR;
            ctx->__g_cc_and_ss_mcu = BGR_to_YCbCr_411_MCU;
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
          {
            jprops->fast_processing = IJL_422_YCBCR_111_BGR;
            ctx->__g_cc_and_ss_mcu = BGR_to_YCbCr_422_MCU;
          }
          else
          {
          }
        }
      }
      else if(jprops->jframe.ncomps == 4)
      {
        /* 4 channel JPEG fast paths supported: */

        if((jprops->DIBColor == IJL_RGBA_FPX) && (jprops->DIBChannels == 4))
        {
          if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1) &&
             (jprops->jframe.comps[3].hsampling == 2) && (jprops->jframe.comps[3].vsampling == 2))
          {
            jprops->fast_processing = IJL_4114_YCBCRA_FPX_1111_RGBA_FPX;
            ctx->__g_cc_and_ss_mcu = RGBA_FPX_to_YCbCrA_FPX_4114_MCU;
          }
          else if
            ((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
             (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
             (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1) &&
             (jprops->jframe.comps[3].hsampling == 2) && (jprops->jframe.comps[3].vsampling == 1))
          {
            jprops->fast_processing = IJL_4224_YCBCRA_FPX_1111_RGBA_FPX;
            ctx->__g_cc_and_ss_mcu = RGBA_FPX_to_YCbCrA_FPX_4224_MCU;
          }
        }
      }
      else /* Number of components is from 1-255 but not either 3 or 4. */
      {
        /* No fast paths supported. */
      }
    }
    else if((jprops->upsampling_reqd) && (!jprops->cconversion_reqd))
    {
      if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 1)
      {
        ctx->__g_cc_and_ss_mcu = SS_General_MCU;
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 3 && jprops->DIBColor != IJL_BGR)
      {
        if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 2) &&
           (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
           (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
        {
          jprops->fast_processing = IJL_411_OTHER_111_OTHER;
          ctx->__g_cc_and_ss_mcu = OTHER_to_OTHER_411_MCU;
        }
        if((jprops->jframe.comps[0].hsampling == 2) && (jprops->jframe.comps[0].vsampling == 1) &&
           (jprops->jframe.comps[1].hsampling == 1) && (jprops->jframe.comps[1].vsampling == 1) &&
           (jprops->jframe.comps[2].hsampling == 1) && (jprops->jframe.comps[2].vsampling == 1))
        {
          if(jprops->DIBSubsampling == IJL_422)
          {
            jprops->fast_processing = IJL_YCBYCR_YCBCR;
            ctx->__g_cc_and_ss_mcu = YCbYCr_to_YCbCr_422_MCU;
          }
          else
          {
            jprops->fast_processing = IJL_422_OTHER_111_OTHER;
            ctx->__g_cc_and_ss_mcu = OTHER_to_OTHER_422_MCU;
          }
        }
      }
      else
      {
        ctx->__g_cc_and_ss_mcu = SS_General_MCU;
      }
    }
    else if(!(jprops->upsampling_reqd) && (jprops->cconversion_reqd)) /* george 08.10.99 */
    {
      if(jprops->jframe.ncomps == 1 && jprops->DIBChannels == 3)
      {
        ctx->__g_cc_and_ss_mcu = CC_RGB_to_YCbCr_General_MCU;
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 3)
      {
        if(jprops->DIBColor == IJL_RGB)
        {
          jprops->fast_processing = IJL_111_YCBCR_111_RGB;
          ctx->__g_cc_and_ss_mcu = RGB_to_YCbCr_111_MCU;
        }
        else if(jprops->DIBColor == IJL_BGR)
        {
          jprops->fast_processing = IJL_111_YCBCR_111_BGR;
          ctx->__g_cc_and_ss_mcu = BGR_to_YCbCr_111_MCU;
        }
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 4)
      {
        ctx->__g_cc_and_ss_mcu = CC_SS_RGB_to_YCbCr_General_MCU;
      }
      else if((jprops->jframe.ncomps == 4) && (jprops->DIBChannels == 4))
      {
        if(jprops->DIBColor == IJL_RGBA_FPX)
        {
          jprops->fast_processing = IJL_1111_YCBCRA_FPX_1111_RGBA_FPX;
          ctx->__g_cc_and_ss_mcu = RGBA_FPX_to_YCbCrA_FPX_1111_MCU;
        }
      }
    }
    else if((!jprops->upsampling_reqd) && (!jprops->cconversion_reqd))
    {
      if(jprops->jframe.ncomps == 1 && jprops->DIBColor == IJL_G && jprops->JPGColor == IJL_G)
      {
        jprops->fast_processing = IJL_1_G_1_G;
        ctx->__g_cc_and_ss_mcu = Y_to_Y_111_MCU;
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 1 && jprops->JPGColor == IJL_YCBCR)
      {
        ctx->__g_cc_and_ss_mcu = Input_Interleave_General_MCU;
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 3)
      {
        if(jprops->DIBColor == IJL_BGR)
        {
          ctx->__g_cc_and_ss_mcu = Input_Interleave_General_MCU;
        }
        else
        {
          jprops->fast_processing = IJL_111_OTHER_111_OTHER;
          ctx->__g_cc_and_ss_mcu = OTHER_to_OTHER_111_MCU;
        }
      }
      else if(jprops->jframe.ncomps == 3 && jprops->DIBChannels == 4)
      {
        ctx->__g_cc_and_ss_mcu = Input_Interleave_General_MCU;
      }
      else if(jprops->jframe.ncomps == 4)
      {
        /* Fast path supported: */
        jprops->fast_processing = IJL_1111_RGBA_FPX_1111_RGBA_FPX;
        ctx->__g_cc_and_ss_mcu = OTHER_to_OTHER_1111_MCU;
      }
      else if(jprops->JPGColor == IJL_OTHER && jprops->DIBColor == IJL_OTHER)
      {
        ctx->__g_cc_and_ss_mcu = Input_Interleave_General_MCU;
      }
    }
  }

Exit:

  return jerr;
} /* Set_Encode_Fast_Path() */

