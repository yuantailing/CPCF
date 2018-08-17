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
#ifndef __FRAME_DECODER_H__
#include "frame_decoder.h"
#endif



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    IJL_Decode
//
//  Purpose
//    Main decoder routine. This routine gets segments from a JPEG
//    datastream and passes control to the routines that acquire data
//    from those segments.
//
//  Context
//    Called from the user-interface routines.
//    This function should be called twice. In the first time,
//    it decodes until it sees the Frame Header when it returns
//    the image info (such as width, height).
//    During the 2nd time, it decodes the rest.
//
//  Returns
//    valid error code or 0 if OK
//
//  Parameters
//    jprops Pointer to IJL properties persistent storage.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) IJL_Decode(
  JPEG_PROPERTIES* jprops)
{
  BOOL   bres;
  STATE* pState = &jprops->state;
  IJLERR jerr   = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in IJL_Decode\n");

  /* Bypass state initalization if this is an interrupted scan. */
  if(NULL == jprops->jscan || jprops->jinterleaveType != 0)
  {
    if(NULL != jprops->jscan)
    {
      if(jprops->jscan->curxMCU == 0 && jprops->jscan->curyMCU == 0)
      {
        DB_Init(pState);
      }
    }
    else
    {
      DB_Init(pState);
    }
  }

  if(jprops->jinterleaveType != 0)
  {
    /* skip if returning from an interrupt. */
    if(NULL != jprops->jscan)
    {
      if(jprops->jscan->curxMCU == 0 && jprops->jscan->curyMCU == 0)
      {
zero_file:
        if((jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
           (jprops->iotype == IJL_JFILE_READENTROPY) ||
           (jprops->iotype == IJL_JFILE_READONEHALF) ||
           (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
           (jprops->iotype == IJL_JFILE_READONEQUARTER))
        {
          pState->entropy_bytes_processed = 0;
          /* The current position in the file is */
          /* given by entropy_bytes_processed    */
          bres = ownSeekFile(pState->file,pState->entropy_bytes_processed,FILE_BEGIN);

          if(FALSE == bres)
          {
            TRACE0(trERROR,"ERROR: seek in file failed\n");
            jerr = IJL_FILE_ERROR;
            goto Exit;
          }

          /* Fill up the decode buffer */
          jerr = Buffer_Read_Bytes(pState);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: read from file failed\n");
            goto Exit;
          }
        }
        else
        {
          /* Must be working from a buffer */
          pState->cur_entropy_ptr         = jprops->JPGBytes;
          pState->entropy_bytes_processed = 0;
          pState->entropy_bytes_left      = pState->entropy_buf_maxsize;
        }
      }
    }
    else
    {
      goto zero_file;
    }
  }

  /* Added tests for scaled modes, too. */
  if(jprops->iotype != IJL_JFILE_READENTROPY    &&
     jprops->iotype != IJL_JBUFF_READENTROPY    &&
     jprops->iotype != IJL_JFILE_READONEHALF    &&
     jprops->iotype != IJL_JBUFF_READONEHALF    &&
     jprops->iotype != IJL_JBUFF_READONEQUARTER &&
     jprops->iotype != IJL_JFILE_READONEQUARTER &&
     jprops->iotype != IJL_JBUFF_READONEEIGHTH  &&
     jprops->iotype != IJL_JFILE_READONEEIGHTH)
  {
    if(jprops->roi.top    == 0 &&
       jprops->roi.bottom == 0 &&
       jprops->roi.right  == 0 &&
       jprops->roi.left   == 0)
    {
      pState->entropy_bytes_processed = 0;
    }
  }

  /* if the frame has been seen, re-initalize color space variables */
  /* if user changed DIB color space between IJL_Read calls.        */
  if(jprops->needframe == 0)
  {
    jerr = Set_Decode_Fast_Path(jprops);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: Set_Decode_Fast_Path() failed\n");
      goto Exit;
    }
  }

  jerr = DP_ParseBitstream(jprops);
  if(IJL_OK > jerr)
    goto Exit;


Exit:

  TRACE0(trCALL|trAFTER,"leave IJL_Decode\n");

  return jerr;
} /* IJL_Decode() */


