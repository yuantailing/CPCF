/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 1998-2006 Intel Corporation. All Rights Reserved.
//
//  Intel(R) Integrated Performance Primitives Intel(R) JPEG Library -
//        Intel(R) IPP Version for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel(R) Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel(R) IPP product
//  installation for more information.
//
//  JPEG is an international standard promoted by ISO/IEC and other organizations.
//  Implementations of these standards, or the standard enabled platforms may
//  require licenses from various entities, including Intel Corporation.
//
*/

#include "precomp.h"
#ifndef __SCAN_DECODER_H__
#include "scan_manager/scan_decoder.h"
#endif


#ifdef __INTEL_COMPILER
#pragma warning(disable:171)
#pragma warning(disable:188)
#endif

#pragma warning(disable:4312)



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    iDct_US_CC
//
//  Purpose:
//    Calls inverse Dct, up-sampling and color conversion functions for MCU
//    after scan decoding
//
//  Returns: IJL_OK for the present;
//
//  Parameters:
//    structures are the same as in other functions,
//    i, j        - horizontal and vertical MCU number,
//    size        - size of MCU buffer in 8x8 blocks,
//    MCUptr      - pointer to buffer (short) for transformation,
//    scalefactor - defines how to reduce dst size,
//    iDCT_instead_of_US - if true, then real up-sampling is replaced by
//                  scale inverse DCT with more dst-size
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) iDct_US_CC(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  SCAN*            scan,
  int              i,
  int              j,
  int              size,
  short*           MCUptr,
  int              numxMCUs,
  int              numyMCUs,
  int              scalefactor,
  int              iDCT_instead_of_US)
{
  int      l, k, m, n, s, scale;
  IJL_RECT MCURoi;
  IJL_CONTEXT* ctx = (IJL_CONTEXT*)state->ctx;

  scale = (scalefactor == 1) ? 8 :
          (scalefactor == 2) ? 4 :
          (scalefactor == 4) ? 2 : 1;

  m = size << 6;

  if((i != numxMCUs) && (j != numyMCUs))
  {
    n = (jprops->progressive_found) ? jprops->jframe.ncomps : scan->ncomps;

    switch(scalefactor)
    {
      case 1:
        for(l = 0; l < size; l++)
        {
          iDCT8x8(MCUptr + (l << 6));
        }

        break;

      case 2:
        if(iDCT_instead_of_US)
        {
          for(k = 0, l = 0; k < n; k++)
          {
            m = (jprops->progressive_found) ?
                jprops->jframe.comps[k].hsampling * jprops->jframe.comps[k].vsampling :
                scan->comps[k].hsampling * scan->comps[k].vsampling;
            if(m == 1)
            {
              iDCT8x8(MCUptr + (l++ << 6));
            }
            else
            {
              for(; m--; )
              {
                iDCT4x4(MCUptr + (l++ << 6));
              }
            }
          }
        }
        else
        {
          for(l = 0; l < size; l++)
          {
            iDCT4x4(MCUptr + (l << 6));
          }
        }

        break;

      case 4:
        if(iDCT_instead_of_US)
        {
          for(k = 0, l = 0; k < n; k++)
          {
            m = (jprops->progressive_found) ?
                jprops->jframe.comps[k].hsampling * jprops->jframe.comps[k].vsampling :
                scan->comps[k].hsampling * scan->comps[k].vsampling;
            if(m == 1)
            {
              iDCT4x4(MCUptr + (l++ << 6));
            }
            else
            {
              for(; m--; )
              {
                iDCT2x2(MCUptr + (l++ << 6));
              }
            }
          }
        }
        else
        {
          for(l = 0; l < size; l++)
          {
            iDCT2x2(MCUptr + (l << 6));
          }
        }

        break;

      case 8:
        if(iDCT_instead_of_US)
        {
          for(k = 0, l = 0; k < n; k++)
          {
            m = (jprops->progressive_found) ?
                jprops->jframe.comps[k].hsampling * jprops->jframe.comps[k].vsampling :
                scan->comps[k].hsampling * scan->comps[k].vsampling;
            if(m == 1)
            {
              iDCT2x2(MCUptr + (l++ << 6));
            }
            else
            {
              for(; m--; )
              {
                iDCT1x1(MCUptr + (l++ << 6));
              }
            }
          }
        }
        else
        {
          for(l = 0; l < size; l++)
          {
            iDCT1x1(MCUptr + (l << 6));
          }
        }

        break;
    } /* end switch */

    if(jprops->raw_coefs)
    {
      /*
      // rewrite sampled data from [][][][] to [][] order
      //                                       [][]
      */
      for(k = 0; k < jprops->jframe.ncomps; k++)
      {
        for(s = 0; s < jprops->jframe.comps[k].vsampling; s++)
        {
          for(m = 0; m < jprops->jframe.comps[k].hsampling; m++)
          {
            unsigned short *cvPtr = jprops->raw_coefs->raw_ptrs[k] +
              ((j * jprops->jframe.comps[k].vsampling + s) * 8 * numxMCUs * jprops->jframe.comps[k].hsampling +
               (i * jprops->jframe.comps[k].hsampling + m))* 8;

            for(l = 0; l < 8; l++, cvPtr += (numxMCUs * jprops->jframe.comps[k].hsampling * 8), MCUptr += 8)
            {
              IPPCALL(ippsCopy_8u)((const Ipp8u*)MCUptr,(Ipp8u*)cvPtr,sizeof(short) << 3);
            }
          }
        }
      }
      return IJL_OK;
    }

  }

  if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
  {
    short *ptrB, *ptrD;

    if(jprops->jframe.max_hsampling == 2 &&
       jprops->jframe.max_vsampling == 1)
    {
      MCURoi.left = jprops->roi.left / (scale * 2);

      if(i != numxMCUs)
      {

        if(i == (numxMCUs-1))
        {
          /* continue rows */
          s = jprops->JPGWidth & 0xf;

          if(s)
          {
            l = scalefactor << 1;
            s = (s + l-1) / l;
            ptrB = (short*)jprops->MCUBuf;

            if(!jprops->jinterleaveType)
            {
              for(n = 0; n < jprops->JPGChannels; n++)
              {
                if(jprops->jframe.comps[n].hsampling == 1)
                {
                  for(k = 0, ptrD = ptrB; k < 8; k++, ptrD += 8)
                  {
                    for(l = s; l < 8; l++)
                    {
                      ptrD[l] = ptrD[l-1];
                    }
                  }
                }
                ptrB += (jprops->jframe.comps[n].hsampling)  * DCTSIZE2;
              }
            }
            else
            {
              if(scan->comps[0].hsampling == 1)
              {
                for(k = 0, ptrD = ptrB; k < 8; k++, ptrD += 8)
                {
                  for(l = s; l < 8; l++)
                  {
                    ptrD[l] = ptrD[l-1];
                  }
                }
              }
            }
          }
        }

        /* store current MCU as right border of previous */
        ptrB = jprops->sampling_state_ptr->cur_row + (i+1) * m;
        ptrD = (short*) jprops -> MCUBuf;
        for(l = m; l--;)
        {
          *ptrB++ = *ptrD++;
        }

        /* left border for 0 MCU */
        if(!i)
        {
          ptrB = jprops->sampling_state_ptr->cur_row + scale-1;
          ptrD = (short*)jprops->MCUBuf;
          for(k = size; k--;)
          {
            for(l = 8; l--; ptrB += 8, ptrD += 8)
            {
              *ptrB = *ptrD;
            }
          }
        }
      } /* i != numxMCUs */

      if(i > MCURoi.left)
      {
        ptrB = jprops->sampling_state_ptr->cur_row + i * m;
        ptrD = (short*)jprops->MCUBuf;
        for(l = m; l-- ;)
        {
          *ptrD++ = *ptrB++;
        }
        i--;
      }
      else
      {
        return IJL_OK;
      }
    }

    if(jprops->jframe.max_hsampling == 2 &&
       jprops->jframe.max_vsampling == 2 &&
       scalefactor == 1)
    {
      short *tptr;

      MCURoi.left = jprops->roi.left >> 4;
      MCURoi.top  = jprops->roi.top  >> 4;

      if(j != jprops->sampling_state_ptr->cur_row_number)
      {
        tptr = jprops->sampling_state_ptr->top_row;
        jprops->sampling_state_ptr->top_row = jprops->sampling_state_ptr->cur_row;
        jprops->sampling_state_ptr->cur_row = jprops->sampling_state_ptr->bottom_row;
        if(j == numyMCUs)
        {
          jprops->sampling_state_ptr->bottom_row = jprops->sampling_state_ptr->last_row;
          jprops->sampling_state_ptr->last_row = tptr;
        }
        else
        {
          jprops->sampling_state_ptr->bottom_row = tptr;
        }
      }

      jprops->sampling_state_ptr->cur_row_number = j;

      if(i != numxMCUs && j != numyMCUs)
      {
        if(i == (numxMCUs-1)) /* right border */
        {
          /* continue rows */
          s = jprops->JPGWidth & 0xf;
          if(s)
          {
            l = scalefactor << 1;
            s = (s + l-1) / l;
            ptrB = (short*)jprops->MCUBuf;

            if(!jprops->jinterleaveType)
            {
              for(n = 0; n < jprops->JPGChannels; n++)
              {
                if(jprops->jframe.comps[n].hsampling == 1)
                {
                  for(k = 0, ptrD = ptrB; k < 8; k++, ptrD += 8)
                  {
                    for(l = s; l < 8; l++)
                    {
                      ptrD[l] = ptrD[l-1];
                    }
                  }
                }
                ptrB += jprops->jframe.comps[n].hsampling *
                        jprops->jframe.comps[n].vsampling * DCTSIZE2;
              }
            }
            else
            {
              if(scan->comps[0].hsampling == 1)
              {
                for(k = 0, ptrD = ptrB; k < 8; k++, ptrD += 8)
                {
                  for(l = s; l < 8; l++)
                  {
                    ptrD[l] = ptrD[l-1];
                  }
                }
              }
            }
          }
        }

        if(j == (numyMCUs-1))
        {
          /* continue columns */
          s = jprops->JPGHeight & 0xf;

          if(s)
          {
            l = scalefactor << 1;
            s = (s + l-1) / l;
            ptrB = (short*)jprops->MCUBuf;

            for(n = 0; n < jprops->JPGChannels; n++)
            {
              if(jprops->jframe.comps[n].vsampling == 1)
              {
                for(k = 0, ptrD = ptrB; k < 8; k++, ptrD++)
                {
                  for(l = s << 3; l < 64; l += 8)
                  {
                    ptrD[l] = ptrD[l-8];
                  }
                }
              }
              ptrB += jprops->jframe.comps[n].hsampling *
                      jprops->jframe.comps[n].vsampling * DCTSIZE2;
            }
          }
        }

        /* store current MCU as bottom border for MCU of previous row */
        ptrB = jprops->sampling_state_ptr->bottom_row + (i + 1) * m;
        ptrD = (Ipp16s*)jprops->MCUBuf;
        IPPCALL(ippsCopy_8u)((const Ipp8u*)ptrD,(Ipp8u*)ptrB,m*sizeof(Ipp16s));

        if(i == 0) /* left border */
        {
          ptrB = jprops->sampling_state_ptr->bottom_row + 7;
          ptrD = (short*)jprops->MCUBuf;
          for(k = size; k--;)
          {
            for(l = 8; l--; ptrB += 8, ptrD += 8)
            {
              *ptrB = *ptrD;
            }
          }
        }

        if(i == (numxMCUs-1)) /* right border */
        {
          ptrB = jprops->sampling_state_ptr->bottom_row + (numxMCUs+1) * m;
          ptrD = (short*)jprops->MCUBuf + 7;
          for(k = size; k--;)
          {
            for(l = 8; l--; ptrB += 8, ptrD += 8)
            {
              *ptrB = *ptrD;
            }
          }
        }

        if(j == 0) /* top border */
        {
          ptrB = jprops->sampling_state_ptr->cur_row + (i + 1) * m;
          ptrD = (short*)jprops->MCUBuf;
          for(k = size; k--;)
          {
            ptrB += 56;
            for(l = 8; l--; ptrD++, ptrB++)
            {
              *ptrB = *ptrD;
            }
            ptrD += 56;
          }

          if(!i) /* top and left */
          {
            ptrB = jprops->sampling_state_ptr->cur_row + 63;
            ptrD = (short*)jprops->MCUBuf;
            for(k = size; k--; ptrD += DCTSIZE2, ptrB += DCTSIZE2)
            {
              *ptrB = *ptrD;
            }
          }

          if(i == (numxMCUs-1)) /* top and right */
          {
            ptrB = jprops->sampling_state_ptr->cur_row + (numxMCUs+1) * m + 56;
            ptrD = (short*)jprops->MCUBuf + 7;
            for(k = size; k--; ptrD += DCTSIZE2, ptrB += DCTSIZE2)
            {
              *ptrB = *ptrD;
            }
          }
        }

        if(j == (numyMCUs-1)) /* bottom border */
        {
          ptrB = jprops->sampling_state_ptr->last_row + (i + 1) * m;
          ptrD = (short*)jprops->MCUBuf;
          for(k = size; k--;)
          {
            ptrD += 56;
            for(l = 8; l--; ptrD++, ptrB++)
            {
              *ptrB = *ptrD;
            }
            ptrB += 56;
          }

          if(!i) /* bottom and left */
          {
            ptrB = jprops->sampling_state_ptr->last_row + 7;
            ptrD = (short*)jprops->MCUBuf + 56;
            for(k = size; k--; ptrD += DCTSIZE2, ptrB += DCTSIZE2)
            {
              *ptrB = *ptrD;
            }
          }

          if(i == (numxMCUs-1)) /* bottom and right */
          {
            ptrB = jprops->sampling_state_ptr->last_row + (numxMCUs + 1) * m;
            ptrD = (short*)jprops->MCUBuf + 63;
            for(k = size; k--; ptrD += DCTSIZE2, ptrB += DCTSIZE2)
            {
              *ptrB = *ptrD;
            }
          }
        }
      }

      if(i > MCURoi.left && j > MCURoi.top)
      {
        ptrB = jprops->sampling_state_ptr->cur_row + i * m;
        ptrD = (short*)jprops->MCUBuf;
        for(l = m; l--;)
        {
          *ptrD++ = *ptrB++;
        }
        i--; j--;
      }
      else
      {
        return IJL_OK;
      }
    }
  }

  if(scalefactor == 1)
  {
    /* perform Color Conversion and upsampling on a whole MCU of blocks */
    ctx->__g_us_and_cc_mcu(jprops, i, j);
  }
  else
  {
    US_And_CC_MCU_Scaled(jprops, state, scan, i, j, scale, iDCT_instead_of_US);
  }

  return IJL_OK;
} /* iDct_US_CC() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Seek_Pos
//
//  Purpose:
//    Modifies the decoder state to correspond to a
//    previously determined decoder state at the
//    beginning of one of the MCU Rows.
//
//  Context:
//    Called from w/in scan decoders dedicated to
//    ROI-based JPEG decoding.
//
//  Returns:
//    None, but the state and scan are modified.
//
//  Parameters:
//    MCURow_number Index specifying MCU Row of the
//                  MCU Row offsets table from which the
//                  current state is to be extracted.
//                  the current state information.
//    *state        Pointer to IJL state variables.
//    *scan         Pointer to current Scan variables.
//    *jprops       Pointer to global IJL properties structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) Seek_Pos(
  int              MCURow_number,
  STATE*           state,
  SCAN*            scan,
  JPEG_PROPERTIES* jprops)
{
  int    size;
  BOOL   bres;
  IJLERR res;

  TRACE0(trCALL|trBEFORE,"enter in Seek_Pos\n");

  state->unread_marker   = jprops->rowoffsets[MCURow_number].unread_marker;

  IPPCALL(ippiDecodeHuffmanStateGetBufSize_JPEG_8u)(&size);

  IPPCALL(ippsCopy_8u)(
    (const Ipp8u*)jprops->rowoffsets[MCURow_number].pDecHuffState,
    (Ipp8u*)state->u.pDecHuffState,
    size);

  scan->dc_diff[0] = jprops->rowoffsets[MCURow_number].dcval1;
  scan->dc_diff[1] = jprops->rowoffsets[MCURow_number].dcval2;
  scan->dc_diff[2] = jprops->rowoffsets[MCURow_number].dcval3;
  scan->dc_diff[3] = jprops->rowoffsets[MCURow_number].dcval4;

  if(jprops->iotype == IJL_JFILE_READWHOLEIMAGE ||
     jprops->iotype == IJL_JFILE_READENTROPY ||
     jprops->iotype == IJL_JFILE_READONEHALF ||
     jprops->iotype == IJL_JFILE_READONEEIGHTH ||
     jprops->iotype == IJL_JFILE_READONEQUARTER)
  {
    state->entropy_bytes_left = 0;
    state->entropy_bytes_processed =
      jprops->rowoffsets[MCURow_number].offset;

    /* The current position in the file is given by entropy_bytes_processed */
    bres = ownSeekFile(state->file,state->entropy_bytes_processed,FILE_BEGIN);
    if(FALSE == bres)
    {
      TRACE0(trERROR,"ERROR: seek in file error\n");
      TRACE0(trCALL|trAFTER,"leave Seek_Pos\n");
      return IJL_FILE_ERROR;
    }

    /* fill up the decode buffer. */
    res = Buffer_Read_Bytes(state);
    if(IJL_OK != res)
    {
      TRACE0(trERROR,"ERROR: read from file error\n");
      TRACE0(trCALL|trAFTER,"leave Seek_Pos\n");
      return IJL_FILE_ERROR;
    }
  }
  else
  {
    /* must be working from a buffer. */
    state->cur_entropy_ptr =
      (Ipp8u*)jprops->rowoffsets[MCURow_number].offset;
    state->entropy_bytes_processed = state->cur_entropy_ptr -
      jprops->JPGBytes;
    state->entropy_bytes_left = state->entropy_buf_maxsize -
      state->entropy_bytes_processed;
  }

  TRACE0(trCALL|trAFTER,"leave Seek_Pos\n");

  return IJL_OK;
} /* Seek_Pos() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    Set_Pos
//
//  Purpose:
//    Persists the state of the decoder at the beginning
//    of a MCURow of data.
//
//  Context:
//    Called from w/in scan decoders dedicated to
//    ROI-based JPEG decoding.
//
//  Returns:
//    None, but the rowoffsets structure is modified.
//
//  Parameters:
//    MCURow_number Index specifying MCU Row of the
//                  MCU Row offsets table which will receive
//                  the current state information.
//    *state        Pointer to IJL state variables.
//    *scan         Pointer to current Scan variables.
//    *jprops       Pointer to global IJL properties structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) Set_Pos(
  int              MCURow_number,
  STATE*           state,
  SCAN*            scan,
  JPEG_PROPERTIES* jprops)
{
  int size;

  jprops->rowoffsets[MCURow_number].dcval1 = scan->dc_diff[0];
  jprops->rowoffsets[MCURow_number].dcval2 = scan->dc_diff[1];
  jprops->rowoffsets[MCURow_number].dcval3 = scan->dc_diff[2];
  jprops->rowoffsets[MCURow_number].dcval4 = scan->dc_diff[3];

  IPPCALL(ippiDecodeHuffmanStateGetBufSize_JPEG_8u)(&size);

  IPPCALL(ippsCopy_8u)(
    (const Ipp8u*)state->u.pDecHuffState,
    (Ipp8u*)jprops->rowoffsets[MCURow_number].pDecHuffState,
    size);

  jprops->rowoffsets[MCURow_number].unread_marker = state->unread_marker;

  if((jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_READONEHALF) ||
     (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
     (jprops->iotype == IJL_JFILE_READENTROPY) ||
     (jprops->iotype == IJL_JFILE_READONEEIGHTH))
  {
    jprops->rowoffsets[MCURow_number].offset = state->entropy_bytes_processed;
  }
  else
  {
    jprops->rowoffsets[MCURow_number].offset = (int)state->cur_entropy_ptr;
  }

  return IJL_OK;
} /* Set_Pos() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Get_Restart
//
//  Purpose
//
//  Context
//
//  Returns
//
//  Parameters
//    state         Pointer to IJL state variables.
//    scan          Pointer to current Scan variables.
//    scan_numxMCUs
//    scan_numyMCUs
//    i
//    j
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) Get_Restart(
  STATE*           pState,
  SCAN*            pScan,
  int              scan_numxMCUs,
  int              scan_numyMCUs,
  int              i,
  int              j)
{
  int    m;
  int    byte0;
  int    byte1;
  IJLERR jerr = IJL_OK;

  if(pState->unread_marker == 0xff)
  {
    for(;;)
    {
      jerr = _READ_BYTE(pState,&byte0);
      if(IJL_OK != jerr)
        goto Exit;

      /* find the last 0xff byte */
      if(byte0 == 0xff)
      {
        jerr = _READ_BYTE(pState,&byte1);
        if(IJL_OK != jerr)
          goto Exit;

        /* find the first non 0xff or non 0 byte */
        if(byte1 != 0xff && byte1 != 0)
        {
          pState->unread_marker = (IJL_MARKER)byte1;
          break;
        }
        else
        {
          pState->cur_entropy_ptr--;
          pState->entropy_bytes_processed--;
          pState->entropy_bytes_left++;
        }
      }
      else
      {
        pState->unread_marker = (IJL_MARKER)byte0;
        break;
      }
    }
  }

  if(!pScan->restart_interv)
  {
    return IJL_OK;
  }

  if((((j * scan_numxMCUs) + i + 1) % pScan->restart_interv) == 0)
  {
    /*
    // Start of a new entropy section.
    // Restart the dc_difference and get ready for a new marker.
    */
    TRACE0(trINFO,"Start of a next restart interval\n");

    /* If at the last restart section, don't do this. */
    if(((j * scan_numxMCUs) + i + 1) < (scan_numxMCUs * scan_numyMCUs))
    {
      pScan->dc_diff[0] = 0;
      pScan->dc_diff[1] = 0;
      pScan->dc_diff[2] = 0;
      pScan->dc_diff[3] = 0;

      IPPCALL(ippiDecodeHuffmanStateInit_JPEG_8u)(pState->u.pDecHuffState);

      if(pState->unread_marker)
      {
        m = pState->unread_marker - MARKER_RST0;
        pState->unread_marker = 0;
      }
      else
      {
        jerr = DP_Get_Next_Marker(pState,(IJL_MARKER*)&m);
        if(IJL_OK != jerr)
        {
          return jerr;
        }

        m -= MARKER_RST0;
      }
    } /* not last restart_interv */
  } /* restart_interv */

Exit:

  return jerr;
} /* Get_Restart() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DecodeScanBaseline
//
//  Purpose
//    Decode a single scan known to contain restart intervals.  Restart
//    intervals require a re-zeroing of the entropy statistics every n
//    MCUs, where n is determined by the Restart Interval Marker. This
//    routine handles selection of iDCT type used during decoding and
//    selection of routines optimized for MMX(TM) technology
//    (the four combinations of these options result in the routine's
//    source length).
//
//  Context
//
//  Returns
//    Valid error code, or 0 for OK.
//
//  Parameters
//    state       Pointer to IJL state variables.
//    scan        Pointer to current Scan variables.
//    jprops      Pointer to global IJL properties structure.
//    scalefactor Performs scaling (in the DCT's and color conversion).
//                Output data is scaled by 1/1, 1/2, or 1/4.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) DecodeScanBaseline(
  STATE*           state,
  SCAN*            scan,
  JPEG_PROPERTIES* jprops,
  int              scalefactor)
{
  long           i;
  int            j, k, l, m, n, jj, v, h;
  int            blkn_per_component[4];
  int            scan_numxMCUs, scan_numyMCUs;
  int            pixwidth, pixheight;
  int            num8x8blks;
  short*         MCUptr;
  Ipp16u*        quant_table[4];
  HUFFMAN_TABLE* ac_huff_table[4];
  HUFFMAN_TABLE* dc_huff_table[4];
  IJLERR         jerr;
  int            iDCT_instead_of_US;

  num8x8blks = 0;

  /* Fill up temporary arrays with per-component information. */
  for(k = 0; k < scan->ncomps; k++)
  {
    ac_huff_table[k]      = scan->comps[k].ac_table;
    dc_huff_table[k]      = scan->comps[k].dc_table;
    quant_table[k]        = scan->comps[k].quant_table->elements;
    blkn_per_component[k] = scan->comps[k].hsampling *
                            scan->comps[k].vsampling;

    num8x8blks += blkn_per_component[k];
  }

  if(scan->ncomps == 1)
  {
    pixwidth  = 8 * jprops->jframe.max_hsampling / scan->comps[0].hsampling;
    pixheight = 8 * jprops->jframe.max_vsampling / scan->comps[0].vsampling;

    scan_numxMCUs = (jprops->JPGWidth  + pixwidth  - 1) / pixwidth;
    scan_numyMCUs = (jprops->JPGHeight + pixheight - 1) / pixheight;

    num8x8blks            = 1;
    blkn_per_component[0] = 1;
  }
  else
  {
    scan_numxMCUs = jprops->numxMCUs;
    scan_numyMCUs = jprops->numyMCUs;
  }

  if(jprops->jframe.max_hsampling == 1 &&
     jprops->jframe.max_vsampling == 1)
  {
    iDCT_instead_of_US = FALSE;
  }
  else
  {
    iDCT_instead_of_US = TRUE;
    for(i = 0; (i < jprops->jframe.ncomps) && iDCT_instead_of_US; i++)
    {
      if(jprops->jframe.comps[i].hsampling != jprops->jframe.comps[i].vsampling)
      {
        iDCT_instead_of_US = FALSE;
      }

      if((jprops->jframe.comps[i].hsampling!= 1) && (jprops->jframe.comps[i].hsampling != 2))
      {
        iDCT_instead_of_US = FALSE;
      }
    }
  }

  m = (jprops->jframe.max_hsampling == 2 && jprops->jframe.max_vsampling <= 2 && jprops->upsampling_type == IJL_TRIANGLE_FILTER) ? 1:0;
  n = (jprops->jframe.max_hsampling == 2 && jprops->jframe.max_vsampling == 2 && jprops->upsampling_type == IJL_TRIANGLE_FILTER) ? 1:0;

  for(j = scan->curyMCU; j < scan_numyMCUs; j++)
  {
    for(i = scan->curxMCU; i < scan_numxMCUs; i++)
    {
      MCUptr = (short*)jprops->MCUBuf;

      for(k = 0; k < scan->ncomps; k++)
      {
        if(jprops->raw_coefs && !jprops->raw_coefs->data_type)
        {
          for(v = 0; v < scan->comps[k].vsampling; v++)
          {
            for(h = 0; h < scan->comps[k].hsampling; h++)
            {
              unsigned short* cvPtr;
              Ipp16s lastDC = (Ipp16s)scan->dc_diff[k];

              jerr = DecodeHuffman8x8(
                state,
                dc_huff_table[k],
                ac_huff_table[k],
                &lastDC,
                MCUptr);

              scan->dc_diff[k] = lastDC;

              if(jerr < IJL_OK)
              {
                return jerr;
              }

              /*
              //                                [][]
              // rewrite block from [][][][] to [][] order
              */
              cvPtr = jprops->raw_coefs->raw_ptrs[k] +
                ((j * scan->comps[k].vsampling + v) * 8 * scan_numxMCUs * scan->comps[k].hsampling +
                 (i * scan->comps[k].hsampling + h))* 8;

              for(jj = 0; jj < 8; jj++, cvPtr += (scan_numxMCUs * scan->comps[k].hsampling * 8), MCUptr += 8)
              {
                IPPCALL(ippsCopy_8u)((const Ipp8u*)MCUptr,(Ipp8u*)cvPtr,sizeof(short) << 3);
              }

            } /* h sampling */
          } /* v sampling */
        } /* raw dct_coefs output */
        else
        {
          for(l = 0; l < blkn_per_component[k]; l++)
          {
            /* Decode + dequantize each block in the MCU. */

            Ipp16s lastDC = (Ipp16s)scan->dc_diff[k];

            jerr = DecodeHuffman8x8(
              state,
              dc_huff_table[k],
              ac_huff_table[k],
              &lastDC,
              MCUptr);

            scan->dc_diff[k] = lastDC;

            if(jerr < IJL_OK)
            {
              return jerr;
            }

            iQnt8x8(MCUptr,quant_table[k]);

            MCUptr += DCTSIZE2;

          } /* for block_per_component */
        }   /* raw data */
      }     /* for num of components */

      jerr = Get_Restart(state,scan,scan_numxMCUs,scan_numyMCUs,i,j);

      if(IJL_OK != jerr)
      {
        return jerr;
      }

      if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
      {
        iDct_US_CC(
          jprops,
          state,
          scan,
          i,
          j,
          num8x8blks,
          (short*)jprops->MCUBuf,
          scan_numxMCUs,
          scan_numyMCUs,
          scalefactor,
          iDCT_instead_of_US);
      }

      /* check for interrupt */
      if(FALSE != jprops->interrupt)
      {
        if((m && !i) || (n && !j))
          continue;

        scan->curxMCU = (i + 1) % (scan_numxMCUs+m);

        if(scan->curxMCU != 0)
        {
          scan->curyMCU = j;
        }
        else
        {
          scan->curyMCU = j + 1;
          if(scan->curyMCU == (scan_numyMCUs+n))
          {
            scan->curyMCU = 0;
            return IJL_OK;
          }
        }

        TRACE0(trINFO,"update ROI\n");
        jprops->roi.right  = i-m;
        jprops->roi.bottom = j-n;
        jprops->roi.left   = i-m;
        jprops->roi.top    = j-n;

        return IJL_INTERRUPT_OK;
      } /* if interrupt      */
    }   /* for scan_numxMCUs */

    if(m && (!jprops->raw_coefs || jprops->raw_coefs->data_type ))
    {
      iDct_US_CC(
        jprops,
        state,
        scan,
        i,
        j,
        num8x8blks,
        (short*)jprops->MCUBuf,
        scan_numxMCUs,
        scan_numyMCUs,
        scalefactor,
        iDCT_instead_of_US);

      /* check for interrupt */
      if(FALSE != jprops->interrupt)
      {
        scan->curxMCU = (i + 1) % (scan_numxMCUs+m);

        if(scan->curxMCU != 0)
        {
          scan->curyMCU = j;
        }
        else
        {
          scan->curyMCU = j + 1;
          if(scan->curyMCU == (scan_numyMCUs+n))
          {
            scan->curyMCU = 0;
            return IJL_OK;
          }
        }

        jprops->roi.right  = i-m;
        jprops->roi.bottom = j-n;
        jprops->roi.left   = i-m;
        jprops->roi.top    = j-n;

        return IJL_INTERRUPT_OK;
      } /* if interrupt */
    }
  } /* for scan_numyMCUs */

  if(n)
  {
    for(i = scan->curxMCU; i < scan_numxMCUs; i++)
    {
      iDct_US_CC(
        jprops,
        state,
        scan,
        i,
        j,
        num8x8blks,
        (short*)jprops->MCUBuf,
        scan_numxMCUs,
        scan_numyMCUs,
        scalefactor,
        iDCT_instead_of_US);

      /* check for interrupt */
      if(FALSE != jprops->interrupt)
      {
        if((m && !i) || (n && !j))
          continue;

        TRACE0(trINFO,"interrupt detected\n");

        scan->curxMCU = (i + 1) % (scan_numxMCUs+m);

        if(scan->curxMCU != 0)
        {
          scan->curyMCU = j;
        }
        else
        {
          scan->curyMCU = j + 1;
          if(scan->curyMCU == (scan_numyMCUs+n))
          {
            scan->curyMCU = 0;
            TRACE0(trCALL|trAFTER,"leave Decode_Scan_Restart\n");
            return IJL_OK;
          }
        }

        jprops->roi.right  = i-m;
        jprops->roi.bottom = j-n;
        jprops->roi.left   = i-m;
        jprops->roi.top    = j-n;

        return IJL_INTERRUPT_OK;
      } /* if interrupt      */
    }   /* for scan_numxMCUs */

    if(m)
    {
      iDct_US_CC(
        jprops,
        state,
        scan,
        i,
        j,
        num8x8blks,
        (short*)jprops->MCUBuf,
        scan_numxMCUs,
        scan_numyMCUs,
        scalefactor,
        iDCT_instead_of_US);

      /* check for interrupt */
      if(FALSE != jprops->interrupt)
      {
        scan->curxMCU = (i + 1) % (scan_numxMCUs+m);

        if(scan->curxMCU != 0)
        {
          scan->curyMCU = j;
        }
        else
        {
          scan->curyMCU = j + 1;
          if(scan->curyMCU == (scan_numyMCUs+n))
          {
            scan->curyMCU = 0;
            return IJL_OK;
          }
        }

        jprops->roi.right  = i-m;
        jprops->roi.bottom = j-n;
        jprops->roi.left   = i-m;
        jprops->roi.top    = j-n;

        return IJL_INTERRUPT_OK;
      } /* if interrupt */
    } /* if(m) */
  } /* if(n) */

  return IJL_OK;
} /* DecodeScanBaseline() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    DecodeScanProgressive
//
//  Purpose:
//    Decode a progressive scan assumed to contain restart
//    intervals. Restart intervals require a re-zeroing
//    of the entropy statistics every n MCUs, where n is
//    determined by the Restart Interval Marker. This
//    routine handles selection of iDCT type used during
//    decoding and selection of routines optimized with
//    MMX(TM) technology (the four combinations of these
//    options result in the routine's source length).
//
//  Context:
//
//  Returns:
//    Valid error code, or 0 for OK.
//
//  Parameters:
//    state       Pointer to IJL state variables.
//    scan        Pointer to current Scan variables.
//    jprops      Pointer to global IJL properties structure.
//    scalefactor Performs scaling (in the DCT's and color conversion).
//                Output data is scaled by 1/1, 1/2, or 1/4.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) DecodeScanProgressive(
  STATE*           state,
  SCAN*            scan,
  JPEG_PROPERTIES* jprops,
  int              scalefactor)
{
  int            i;
  int            j, k, l, m, jj, mm, nn;
  int            scan_numxMCUs, scan_numyMCUs;
  int            blkn_per_component[4];
  short*         MCUptr;
  short*         coef_buf;
  IJLERR         jerr;
  HUFFMAN_TABLE* ac_huff_table[4];
  HUFFMAN_TABLE* dc_huff_table[4];
  int            iDCT_instead_of_US;
  int num8x8blks = 0;
  int size       = 0;


  TRACE0(trCALL|trBEFORE,"enter in DecodeScanProgressive()\n");

  /* Fill up temporary arrays with per-component information. */
  for(k = 0; k < scan->ncomps; k++)
  {
    ac_huff_table[k] = scan->comps[k].ac_table;
    dc_huff_table[k] = scan->comps[k].dc_table;

    blkn_per_component[k] = scan->comps[k].hsampling *
                            scan->comps[k].vsampling;

    num8x8blks += blkn_per_component[k];
  }

  /*
  // From this point forward is the only code touched if no
  // restart intervals are defined.  For the last restart
  // interval (complete the scan).
  */

  for(k = 0; k < jprops->jframe.ncomps; k++)
  {
    size += (jprops->jframe.comps[k].hsampling *
             jprops->jframe.comps[k].vsampling);
  }


  if((jprops->jframe.max_hsampling == 1) && (jprops->jframe.max_vsampling == 1))
  {
    iDCT_instead_of_US = FALSE;
  }
  else
  {
    iDCT_instead_of_US = TRUE;
    for(i = 0; (i < jprops->jframe.ncomps) && iDCT_instead_of_US; i++)
    {
      if(jprops->jframe.comps[i].hsampling != jprops->jframe.comps[i].vsampling)
      {
        iDCT_instead_of_US = FALSE;
      }

      if((jprops->jframe.comps[i].hsampling != 1) && (jprops->jframe.comps[i].hsampling != 2))
      {
        iDCT_instead_of_US = FALSE;
      }
    }
  }

  mm = (jprops->jframe.max_hsampling == 2 && jprops->jframe.max_vsampling <= 2
                                          && jprops->upsampling_type==IJL_TRIANGLE_FILTER) ? 1:0;
  nn = (jprops->jframe.max_hsampling == 2 && jprops->jframe.max_vsampling == 2
                                          && jprops->upsampling_type==IJL_TRIANGLE_FILTER) ? 1:0;

  if(scan->start_spec != 0 && scan->end_spec != 0)
  {
    TRACE0(trINFO,"decoding AC scan...\n");
    /* AC coefficients. */

    i  = 8 * jprops->jframe.max_hsampling / scan->comps[0].hsampling;
    j  = 8 * jprops->jframe.max_vsampling / scan->comps[0].vsampling;

    scan_numxMCUs = (jprops->JPGWidth  + i-1) / i;
    scan_numyMCUs = (jprops->JPGHeight + j-1) / j;

    for(j = scan->curyMCU; j < jprops->numyMCUs; j++)
    {
      /* AC Scan. */
      for(jj = 0; jj < scan->comps[0].vsampling; jj++)
      {
        /* Skip the last block row of the image if greater than the image height. */
        if(((j*scan->comps[0].vsampling*8) + (jj*8)) >= jprops->JPGHeight)
          break;

        for(i = scan->curxMCU; i < jprops->numxMCUs; i++)
        {
          coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs))) ;

          /* Skip over any relevant components. */
          for(k = 0; k < scan->comps[0].comp; k++)
          {
            coef_buf += (DCTSIZE2 * jprops->jframe.comps[k].hsampling *
                                    jprops->jframe.comps[k].vsampling);
          }

          /* Skip over relevant 8x8 blocks from this component. */
          coef_buf += (jj * DCTSIZE2 * scan->comps[0].hsampling);

          for(m = 0; m < scan->comps[0].hsampling; m++)
          {
            /* Ignore the last column(s) of the image. */
            if(((i*scan->comps[0].hsampling*8) + (m*8)) >= jprops->JPGWidth)
              break;

            if(scan->approx_high == 0)
            {
              /* AC Coefficient pass:  original. */
              Decode_Block_Prog_AC(
                state,
                coef_buf,
                ac_huff_table[0],
                scan->start_spec,
                scan->end_spec,
                scan->approx_low);

              coef_buf += DCTSIZE2;
            }
            else
            {
              Decode_Block_Prog_AC_SA(
                state,
                coef_buf,
                ac_huff_table[0],
                scan->start_spec,
                scan->end_spec,
                scan->approx_low);

              coef_buf += DCTSIZE2;
            }

            jerr = Get_Restart(
              state,
              scan,
              scan_numxMCUs,
              scan_numyMCUs,
              (i * scan->comps[0].hsampling) + m,
              (j * scan->comps[0].vsampling) + jj);

            if(IJL_OK != jerr)
            {
              TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
              TRACE0(trCALL|trAFTER,"leave Decode_Scan_Restart_Progressive()\n");
              return IJL_BAD_RST_MARKER;
            }
          } /* for m < scan->comps[0].hsampling */
        } /* for i < jprops->numxMCUs */
      } /* for jj < scan->comps[0].vsampling */

      /*
      // Process a whole MCUrow of data.
      // Following two lines are speed improvements when
      // not displaying per-scan information.
      // REMOVE THEM FOR VERSION 2.0.
      */
      if((jprops->jframe.SeenAllDCScans >= jprops->jframe.ncomps) &&
         (jprops->jframe.SeenAllACScans >= jprops->jframe.ncomps))
      {
        short* temp_mcu_ptr;
        if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
        {
          for(i = 0; i < jprops->numxMCUs; i++)
          {
            coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

            MCUptr = (short*)jprops->MCUBuf;

            /* Copy stuff from coef_buf into MCUBuf. */
            IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)MCUptr,size*DCTSIZE2*sizeof(Ipp16s));

            /* Dequantization AC coefficients */
            temp_mcu_ptr = MCUptr;
            for(m = 0; m < jprops->jframe.ncomps; m++)
            {
              int blks = jprops->jframe.comps[m].hsampling * jprops->jframe.comps[m].vsampling;
              for(l = 0; l < blks; l++)
              {
                iQnt8x8(
                  temp_mcu_ptr,
                  jprops->jFmtQuant[jprops->jframe.comps[m].quant_sel].elements);
                temp_mcu_ptr += DCTSIZE2;
              }
            }

            iDct_US_CC(
              jprops,
              state,
              scan,
              i,
              j,
              size,
              MCUptr,
              jprops->numxMCUs,
              jprops->numyMCUs,
              scalefactor,
              iDCT_instead_of_US);
          }
        }
        else
        {
          for(i = 0; i < jprops->numxMCUs; i++)
          {
            coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

            for(k = 0; k < jprops->jframe.ncomps; k++)
            {
              for(jj = 0; jj < jprops->jframe.comps[k].vsampling; jj++)
              {
                for(m = 0; m < jprops->jframe.comps[k].hsampling; m++)
                {
                  unsigned short* tmpPtr = jprops->raw_coefs->raw_ptrs[k] +
                  (j * jprops->numxMCUs * jprops->jframe.comps[k].hsampling * jprops->jframe.comps[k].vsampling * DCTSIZE2) +
                  (jj * DCTSIZE2 * jprops->jframe.comps[k].hsampling * jprops->numxMCUs) +
                  (i * jprops->jframe.comps[k].hsampling * 8) + m * 8;
                  for(l = 0; l < 8; l++, tmpPtr += (jprops->numxMCUs * jprops->jframe.comps[k].hsampling * 8), coef_buf += 8)
                  {
                    IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)tmpPtr,sizeof(short) << 3);
                  }
                } /* for m */
              } /* for jj */
            } /* for k */
          } /* for i */
        } /* if raw_coefs */

        if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
        {
          iDct_US_CC(
            jprops,
            state,
            scan,
            i,
            j,
            size,
            (short*)jprops->MCUBuf,
            jprops->numxMCUs,
            jprops->numyMCUs,
            scalefactor,
            iDCT_instead_of_US);
        }
      } /* if speed improvement */

      if(FALSE != jprops->interrupt)
      {
        scan->curyMCU = j + 1;
        scan->curxMCU = 0;
        if(scan->curyMCU == (jprops->numyMCUs+nn))
        {
          scan->curyMCU = 0;
          TRACE0(trCALL|trAFTER,"leave DecodeScanProgressive()\n");
          return IJL_OK;
        }
        jprops->roi.right  = jprops->numxMCUs - 1;
        jprops->roi.bottom = j;
        jprops->roi.left   = jprops->numxMCUs - 1;
        jprops->roi.top    = j;
        TRACE0(trCALL|trAFTER,"leave DecodeScanProgressive()\n");
        return IJL_INTERRUPT_OK;
      } /* if interrupt       */
    } /* for jprops->numyMCUs */

    if(!jprops->raw_coefs || jprops->raw_coefs->data_type )
    {
      for(i = 0; i < jprops->numxMCUs+1; i++)
      {
        iDct_US_CC(
          jprops,
          state,
          scan,
          i,
          j,
          size,
          (short*)jprops->MCUBuf,
          jprops->numxMCUs,
          jprops->numyMCUs,
          scalefactor,
          iDCT_instead_of_US);
      }
    }
  }
  else /* if((scan->start_spec != 0) && (scan->end_spec != 0)) */
  {
    TRACE0(trINFO,"decoding DC scan...\n");
    /* DC Scan. Won't work if the DC components aren't interleaved. */
    for(j = scan->curyMCU; j < jprops->numyMCUs; j++)
    {
      for(i = scan->curxMCU; i < jprops->numxMCUs; i++)
      {
        coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

        /* Skip over any relevant components. */
        for(k = 0; k < scan->comps[0].comp; k++)
        {
          coef_buf += (DCTSIZE2 * jprops->jframe.comps[k].hsampling * jprops->jframe.comps[k].vsampling);
        }

        MCUptr = (short*)jprops->MCUBuf;

        if(scan->approx_high == 0)
        {
          Ipp16s lastDC;
          /* First DC scan.  No prior scans have been seen. */
          for(k = 0; k < scan->ncomps; k++)
          {
            for(l = 0; l < blkn_per_component[k]; l++)
            {
              /* Zero the coefficient buffer. */
              IPPCALL(ippsZero_8u)((Ipp8u*)coef_buf,DCTSIZE2*sizeof(short));

              lastDC = (short)scan->dc_diff[k];

              /* Decode each block in the MCU. */
              Decode_Block_Prog_DC(
                state,
                dc_huff_table[k],
                coef_buf,
                &lastDC,
                scan->approx_low);

              scan->dc_diff[k] = lastDC;

              coef_buf += DCTSIZE2;
            }
          }
        }
        else /* if(scan->approx_high == 0) */
        {
          for(k = 0; k < scan->ncomps; k++)
          {
            /* Successive approximation on the DC band. */
            for(l = 0; l < blkn_per_component[k]; l++)
            {
              /* Decode the DC coefficient. */
              Decode_Block_Prog_DC_SA(state,coef_buf,scan->approx_low);

              coef_buf += DCTSIZE2;
            }
          }
        } /* if(scan->approx_high == 0) */

        jerr = Get_Restart(state,scan,jprops->numxMCUs,jprops->numyMCUs,i,j);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
          TRACE0(trCALL|trAFTER,"leave Decode_Scan_Restart_Progressive()\n");
          return IJL_BAD_RST_MARKER;
        }

        if((jprops->jframe.SeenAllDCScans >= jprops->jframe.ncomps) &&
           (jprops->jframe.SeenAllACScans >= jprops->jframe.ncomps))
        {

          coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

          if(!jprops->raw_coefs || !jprops->raw_coefs->data_type)
          {
            short* temp_mcu_ptr;
            MCUptr = (short*)jprops->MCUBuf;

            /* Copy stuff from coef_buf into MCUBuf. */
            IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)MCUptr,size*DCTSIZE2*sizeof(short));

            /* Dequantization DC coefficients */
            temp_mcu_ptr = MCUptr;
            for(m = 0; m < jprops->jframe.ncomps; m++)
            {
              int blks = jprops->jframe.comps[m].hsampling * jprops->jframe.comps[m].vsampling;
              for(l = 0; l < blks; l++)
              {
                iQnt8x8(
                  temp_mcu_ptr,
                  jprops->jFmtQuant[jprops->jframe.comps[m].quant_sel].elements);

                temp_mcu_ptr += DCTSIZE2;
              }
            }

            iDct_US_CC(
              jprops,
              state,
              scan,
              i,
              j,
              size,
              MCUptr,
              jprops->numxMCUs,
              jprops->numyMCUs,
              scalefactor,
              iDCT_instead_of_US);
          }
          else
          {
            for(k = 0; k < jprops->jframe.ncomps; k++)
            {
              for(jj = 0; jj < jprops->jframe.comps[k].vsampling; jj++)
              {
                for(m = 0; m < jprops->jframe.comps[k].hsampling; m++)
                {
                  unsigned short* tmpPtr = jprops->raw_coefs->raw_ptrs[k] +
                  (j * jprops->numxMCUs * jprops->jframe.comps[k].hsampling *
                                          jprops->jframe.comps[k].vsampling * DCTSIZE2)  +
                  (jj * DCTSIZE2 * jprops->jframe.comps[k].hsampling * jprops->numxMCUs) +
                  (i * jprops->jframe.comps[k].hsampling * 8) + m * 8;
                  for(l = 0; l < 8; l++, tmpPtr += (jprops->numxMCUs * jprops->jframe.comps[k].hsampling * 8), coef_buf += 8)
                  {
                    IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)tmpPtr,sizeof(short) << 3);
                  }
                } /* for m */
              } /* for jj */
            } /* for k */
          }
        }

        /* process interrupt */
        if(FALSE != jprops->interrupt)
        {
          if((mm && !i) || (nn && !j))
            continue;

          TRACE0(trINFO,"interrupt detected\n");

          scan->curxMCU = (i + 1) % (jprops->numxMCUs+mm);
          if(scan->curxMCU != 0)
          {
            scan->curyMCU = j;
          }
          else
          {
            scan->curyMCU = j + 1;
            if(scan->curyMCU == (jprops->numyMCUs+nn))
            {
              scan->curyMCU = 0;
              TRACE0(trCALL|trAFTER,"leave Decode_Scan_Restart_Progressive\n");
              return IJL_OK;
            }
          }

          TRACE0(trINFO,"update ROI\n");
          jprops->roi.right  = i-mm;
          jprops->roi.bottom = j-nn;
          jprops->roi.left   = i-mm;
          jprops->roi.top    = j-nn;
          TRACE1(trINFO,"jprops->roi.right  = %d\n",jprops->roi.right);
          TRACE1(trINFO,"jprops->roi.bottom = %d\n",jprops->roi.bottom);
          TRACE1(trINFO,"jprops->roi.left   = %d\n",jprops->roi.left);
          TRACE1(trINFO,"jprops->roi.top    = %d\n",jprops->roi.top);

          TRACE0(trCALL|trAFTER,"leave Decode_Scan_Restart_Progressive\n");

          return IJL_INTERRUPT_OK;
        } /* if interrupt */
      } /* for jprops->numxMCUs */

      if(mm)
      {
        iDct_US_CC(
          jprops,
          state,
          scan,
          i,
          j,
          size,
          (short*)jprops->MCUBuf,
          jprops->numxMCUs,
          jprops->numyMCUs,
          scalefactor,
          iDCT_instead_of_US);

        /* process interrupt */
        if(FALSE != jprops->interrupt)
        {
          if((mm && !i) || (nn && !j))
            continue;

          TRACE0(trINFO,"interrupt detected\n");

          scan->curxMCU = (i + 1) % (jprops->numxMCUs+mm);

          if(scan->curxMCU != 0)
            scan->curyMCU = j;
          else
            scan->curyMCU = j + 1;

          if(scan->curyMCU == (jprops->numyMCUs+nn))
          {
            scan->curyMCU = 0;
            TRACE0(trCALL|trAFTER,"leave Decode_Scan_Restart_Progressive\n");
            return IJL_OK;
          }

          TRACE0(trINFO,"update ROI\n");
          jprops->roi.right  = i-mm;
          jprops->roi.bottom = j-nn;
          jprops->roi.left   = i-mm;
          jprops->roi.top    = j-nn;
          TRACE1(trINFO,"jprops->roi.right  = %d\n",jprops->roi.right);
          TRACE1(trINFO,"jprops->roi.bottom = %d\n",jprops->roi.bottom);
          TRACE1(trINFO,"jprops->roi.left   = %d\n",jprops->roi.left);
          TRACE1(trINFO,"jprops->roi.top    = %d\n",jprops->roi.top);

          TRACE0(trCALL|trAFTER,"leave DecodeScanProgressive\n");

          return IJL_INTERRUPT_OK;
        } /* if interrupt */
      }
    } /* jprops->numyMCUs */

    if(nn)
    {
      for(i = scan->curxMCU; i <= jprops->numxMCUs; i++)
      {
        iDct_US_CC(
          jprops,
          state,
          scan,
          i,
          j,
          size,
          (short*)jprops->MCUBuf,
          jprops->numxMCUs,
          jprops->numyMCUs,
          scalefactor,
          iDCT_instead_of_US);

        /* process interrupt */
        if(FALSE != jprops->interrupt)
        {
          if((mm && !i) || (nn && !j))
            continue;

          TRACE0(trINFO,"interrupt detected\n");

          scan->curxMCU = (i + 1) % (jprops->numxMCUs+mm);

          if(scan->curxMCU != 0)
          {
            scan->curyMCU = j;
          }
          else
          {
            scan->curyMCU = j + 1;
            if(scan->curyMCU == (jprops->numyMCUs+nn))
            {
              scan->curyMCU = 0;
              TRACE0(trCALL|trAFTER,"leave DecodeScanProgressive\n");
              return IJL_OK;
            }
          }

          TRACE0(trINFO,"update ROI\n");
          jprops->roi.right  = i-mm;
          jprops->roi.bottom = j-nn;
          jprops->roi.left   = i-mm;
          jprops->roi.top    = j-nn;
          TRACE1(trINFO,"jprops->roi.right  = %d\n",jprops->roi.right);
          TRACE1(trINFO,"jprops->roi.bottom = %d\n",jprops->roi.bottom);
          TRACE1(trINFO,"jprops->roi.left   = %d\n",jprops->roi.left);
          TRACE1(trINFO,"jprops->roi.top    = %d\n",jprops->roi.top);

          TRACE0(trCALL|trAFTER,"leave DecodeScanProgressive\n");
          return IJL_INTERRUPT_OK;
        } /* if interrupt */
      }
    }
  } /* if((scan->start_spec != 0) && (scan->end_spec != 0)) */

  TRACE0(trCALL|trAFTER,"leave DecodeScanProgressive()\n");

  return IJL_OK;
} /* DecodeScanProgressive() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DecodeScanBaselineRoi
//
//  Purpose
//    Decode a Region of Interest (ROI) from a scan.
//    The ROI is given by jprops->roi, and is
//    restricted to lie on MCU boundaries (for safety,
//    assume it must lie on 32-pixel boundaries).
//    The technique used to decode a ROI is as follows:
//
//    A block-interleaved JPEG image is composed of a
//    number of rows of MCUs (called MCU Rows).
//    These blocks can have heights of 8, 16, 24, or
//    32 pixels, depending on the vertical subsampling
//    specified in the JPEG stream.  This code isolates
//    the location of the beginning of each of these
//    rows in the input buffer or file and records these
//    locations in an array (rowoffsets, allocated in
//    Decode_Scan).
//
//    To decode a ROI, start with a JPEG image split
//    into MCU Rows.  The JPEG image has not been
//    accessed, so the starting locations of the MCU
//    Rows within the JPEG stream are unknown.
//    The file looks like the following:
//
//    MCU 1   MCU 2   MCU 3   MCU 4   MCU 5   MCU 6   MCU 7   MCU 8  ... etc.
//    ---------------------------------------------------------------
//    |   |   |   |   |   |   |   |   |
//    |   |   |   |   |   |   |   |   |  MCU Row 1
//    |   |   |   |   |   |   |   |   |
//     ---------------------------------------------------------------
//    |   |   |///////|///////|   |   |   |   |
//    |   |   |///////|///////|   |   |   |   |  MCU Row 2
//    |   |   |///////|///////|   |   |   |   |
//     ---------------------------------------------------------------
//    |   |   |///////|///////|   |   |   |   |
//    |   |   |///////|///////|   |   |   |   |  MCU Row 3
//    |   |   |///////|///////|   |   |   |   |
//     ---------------------------------------------------------------
//    . . . etc.
//
//    The ROI is shown shaded.
//    (note that the ROI edges have to lie on MCU boundaries)
//
//    The state at (MCU1, MCU Row 1) is known implicitly -
//    call it "STATE 1".  From this point, "dummy" reads of
//    the compressed entropy data are read until the start of
//    MCU Row 2 is executed.  (The reads are "Dummy" because
//    the entropy data is decoded but not processed -
//    this significantly speeds the decode of the JPEG
//    compressed data).  The state of the JPEG decoder
//    (number of bytes from the entropy stream decoded,
//    Number of bits buffered, etc.) is now "STATE 2".
//
//        MCU 1  MCU 2  MCU 3  MCU 4  MCU 5  MCU 6  MCU 7  MCU 8 ... etc
//       ---------------------------------------------------------------
//          |   |   |   |   |   |   |   |   |
//  STATE 1 | X | X | X | X | X | X | X | X |  MCU Row 1
//          |   |   |   |   |   |   |   |   |
//       ---------------------------------------------------------------
//          |   |   |///////|///////|   |   |   |   |
//  STATE 2 |   |   |///////|///////|   |   |   |   |  MCU Row 2
//          |   |   |///////|///////|   |   |   |   |
//       ---------------------------------------------------------------
//          |   |   |///////|///////|   |   |   |   |
//          |   |   |///////|///////|   |   |   |   |  MCU Row 3
//          |   |   |///////|///////|   |   |   |   |
//       ---------------------------------------------------------------
//      . . . etc.
//
//    The state of the decoder at the beginning of an MCU Row
//    is stored in an array as soon as a new MCU row is
//    decoded.  Thus STATE 1 and STATE 2  are written
//    as the first and second elements of jprops->rowoffsets.
//
//    The "dummy" decoding of MCU Blocks occurs until the
//    start of MCU3 in MCU Row 2.  Now "real" decoding
//    occurs, with dequantization, iDCT's, and
//    postprocessing.  The results of decoding MCU3 and
//    MCU4 are sent to the output DIB.  Then "dummy"
//    decoding resumes until the start of MCU Row 3:
//
//            MCU 1  MCU 2  MCU 3  MCU 4  MCU 5  MCU 6  MCU 7  MCU 8 ... etc
//           ---------------------------------------------------------------
//          |   |   |   |   |   |   |   |   |
//  STATE 1 | X | X | X | X | X | X | X | X |  MCU Row 1
//          |   |   |   |   |   |   |   |   |
//           ---------------------------------------------------------------
//          |   |   |///////|///////|   |   |   |   |
//  STATE 2 | X | X |///X///|///X///| X | X | X | X |  MCU Row 2
//          |   |   |///////|///////|   |   |   |   |
//           ---------------------------------------------------------------
//          |   |   |///////|///////|   |   |   |   |
//  STATE 3 |   |   |///////|///////|   |   |   |   |  MCU Row 3
//          |   |   |///////|///////|   |   |   |   |
//           ---------------------------------------------------------------
//          . . . etc.
//
//    When STATE 3 is written to the jprops->rowoffsets
//    array.  This process continues, decoding the ROI MCU's
//    to the output DIB and writing STATE X at the beginning
//    of MCU Row X, until the ROI is decoded.
//
//    Subsequent reads can be much faster.
//
//    Suppose the user wishes to decode another ROI:
//
//            MCU 1  MCU 2  MCU 3  MCU 4  MCU 5  MCU 6  MCU 7  MCU 8 ... etc
//           ---------------------------------------------------------------
//          |   |   |   |   |   |   |   |   |
//          |   |   |   |   |   |   |   |   |  MCU Row 1
//          |   |   |   |   |   |   |   |   |
//           ---------------------------------------------------------------
//          |   |///////|   |   |   |   |   |   |
//  STATE 2 | X |///////|   |   |   |   |   |   |  MCU Row 2
//          |   |///////|   |   |   |   |   |   |
//           ---------------------------------------------------------------
//          |   |   |   |   |   |   |   |   |
//          |   |   |   |   |   |   |   |   |  MCU Row 3
//          |   |   |   |   |   |   |   |   |
//           ---------------------------------------------------------------
//          . . . etc.
//
//    The decoder now notices that STATE 2 has been defined
//    and immediately seeks (or offsets, in a buffered
//    read) to the entropy location given in STATE 2.
//    The user executes "dummy" reads for MCU1 (and any
//    other intervening MCU's on the left side of the ROI
//    and then does "real" reads on the ROI MCUs.
//    This saves doing "dummy" reads for the previously
//    decoded MCU rows and for the MCU columns following
//    the ROI MCUs.
//
//  Extensions
//    This technique can be extended (for large images)
//    by using a 2-D State persistence array.
//    For example, there might be a column of states
//    at the left-hand edge of the JPEG image, one at
//    MCU Column #100, etc.  This would give a
//    subsequent-access time to the JPEG file a
//    maximum value of 100 dummy MCU reads once the file
//    had been initially decoded.
//
//    This technique can work for planar
//    (scan-interleaved) JPEG files by keeping track of
//    multiple states for each MCU column (one state
//    per scan), plus information about the offset to
//    the start of each scan.
//
//  Context
//
//  Returns
//    Valid error code, or IJL_ERR_OK if OK.
//
//  Parameters
//    *state    Pointer to IJL state variables.
//    *scan     Pointer to current Scan variables.
//    *jprops   Pointer to global IJL properties structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DecodeScanBaselineRoi
//
//  Purpose
//    Decodes ROI from JPEG data
//
//  Context
//
//  Returns
//    valid error code, or IJL_ERR_OK if ok.
//
//  Parameters
//    *state    IJL state variables.
//    *scan     Current Scan variables.
//    *jprops   IJL variables structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) DecodeScanBaselineRoi(
  STATE*           state,
  SCAN*            scan,
  JPEG_PROPERTIES* jprops,
  int              scalefactor)
{
  int            i, j, k, l, m, n, h, v, jj;
  Ipp16u*        quant_table[4];
  HUFFMAN_TABLE* ac_huff_table[4];
  HUFFMAN_TABLE* dc_huff_table[4];
  short*         MCUptr;
  int            blkn_per_component[4];
  IJLERR         jerr;
  int            scan_numxMCUs, scan_numyMCUs;
  int            pixheight, pixwidth;
  int            rowGroup;
  IJL_RECT       MCURoi;
  int            iDCT_instead_of_US;
  int num8x8blks = 0;
  int topMCURow;

  TRACE0(trCALL|trBEFORE,"enter in DecodeScanBaselineRoi()\n");

  /* Fill up temporary arrays with per-component information. */
  for(k = 0; k < scan->ncomps; k++)
  {
    if(!(jprops->jscan) && (jprops->jinterleaveType != 1))
      scan->dc_diff[k] = 0;

    ac_huff_table[k] = scan->comps[k].ac_table;
    dc_huff_table[k] = scan->comps[k].dc_table;
    quant_table[k]   = scan->comps[k].quant_table->elements;

    blkn_per_component[k] = scan->comps[k].hsampling *
                            scan->comps[k].vsampling;

    num8x8blks           += blkn_per_component[k];
  }

  if(scan->ncomps == 1)
  {
    pixwidth  = 8 * jprops->jframe.max_hsampling / scan->comps[0].hsampling;
    pixheight = 8 * jprops->jframe.max_vsampling / scan->comps[0].vsampling;
    rowGroup  = pixheight * jprops->DIBLineBytes;

    scan_numxMCUs = (jprops->JPGWidth + pixwidth-1) / (pixwidth);
    scan_numyMCUs = (abs(jprops->JPGHeight) + pixheight - 1) / pixheight;

    num8x8blks            = 1;
    blkn_per_component[0] = 1;
  }
  else
  {
    pixwidth  = 8 * jprops->jframe.max_hsampling;
    pixheight = 8 * jprops->jframe.max_vsampling;
    rowGroup  = 8 * jprops->jframe.max_vsampling * jprops->DIBLineBytes;

    /* Calc MCU parameters. */
    scan_numxMCUs = jprops->numxMCUs;
    scan_numyMCUs = jprops->numyMCUs;
  }

  /* to improve any scale reading with "square" upsampling */
  if((jprops->jframe.max_hsampling == 1) && (jprops->jframe.max_vsampling == 1))
  {
    iDCT_instead_of_US = FALSE;
  }
  else
  {
    iDCT_instead_of_US = TRUE;
    for(i = 0; (i < jprops->jframe.ncomps) && iDCT_instead_of_US; i++)
    {
      if(jprops->jframe.comps[i].hsampling != jprops->jframe.comps[i].vsampling)
      {
        iDCT_instead_of_US = FALSE;
      }

      if((jprops->jframe.comps[i].hsampling != 1)&&(jprops->jframe.comps[i].hsampling != 2))
      {
        iDCT_instead_of_US = FALSE;
      }
    }
  }

  MCURoi.left   = jprops->roi.left / (pixwidth / scalefactor);
  MCURoi.right  = (jprops->roi.right + (pixwidth / scalefactor) - 1) /
    (pixwidth / scalefactor);
  MCURoi.top    = jprops->roi.top / (pixheight / scalefactor);
  MCURoi.bottom = (jprops->roi.bottom + (pixheight / scalefactor) - 1) /
    (pixheight/scalefactor);

  m = ((jprops->upsampling_type == IJL_TRIANGLE_FILTER) &&
       (jprops->jframe.max_hsampling==2)) ? 1:0;
  n = ((jprops->upsampling_type == IJL_TRIANGLE_FILTER) &&
       (jprops->jframe.max_vsampling==2)) ? 1:0;

  if(MCURoi.left)
    MCURoi.left -= m;

  if(MCURoi.top)
    MCURoi.top -= n;

  if(MCURoi.right < scan_numxMCUs)
    MCURoi.right += m;

  if(MCURoi.bottom < scan_numyMCUs)
    MCURoi.bottom += n;

  /* Go to the start of the MCURoi. */
  topMCURow = max((scan->curyMCU), MCURoi.top);

  /* If file is multi-scan. */
  if((jprops->jinterleaveType == 1) && (state->cur_scan_comp != 0))
  {
    /* 0. Decode blocks until we get to the */
    /* beginning of the MCUroi column. */
    MCUptr = (short*)jprops->MCUBuf;

    if(scan->curxMCU == 0)
    {
      for(j = 0; j < topMCURow; j++)
      {
        for(i = 0; i < scan_numxMCUs; i++)
        {
          for(k = 0; k < scan->ncomps; k++)
          {
            for(l = 0; l < blkn_per_component[k]; l++)
            {
              short lastDC = (short)scan->dc_diff[k];

              jerr = DecodeHuffman8x8(
                state,
                dc_huff_table[k],
                ac_huff_table[k],
                &lastDC,
                MCUptr);

              scan->dc_diff[k] = lastDC;

              if(IJL_OK > jerr)
              {
                TRACE0(trERROR,"ERROR: __g_decode_block_thumb() failed\n");
                TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
                return IJL_ERR_DATA;
              }
            }
          }

          jerr = Get_Restart(state, scan, scan_numxMCUs, scan_numyMCUs, i, j);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
            TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
            return IJL_BAD_RST_MARKER;
          }
        } /* for scan_numxMCUs */
      } /* for topMCURow */
    } /* if(scan->curxMCU == 0) */
  }
  else /* if((jprops->jinterleaveType == 1) && (state->cur_scan_comp != 0)) */
  {
    if(scan->curxMCU == 0)
    {
      if(jprops->rowoffsets[topMCURow].offset == 0xFFFFFFFF)
      {
        /* Go to the closest defined rowoffset. */
        j = topMCURow;
        while(jprops->rowoffsets[j].offset == 0xFFFFFFFF)
        {
          j--;
        }

        jerr = Seek_Pos(j, state, scan, jprops);
        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: Seek_Pos() failed\n");
          TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
          return jerr;
        }

        /* Decode the intervening rows until we get to the */
        /* top edge of the MCU ROI. */
        MCUptr = (short*)jprops->MCUBuf;

        for(; j < topMCURow; j++)
        {
          for(i = 0; i < scan_numxMCUs; i++)
          {
            for(k = 0; k < scan->ncomps; k++)
            {
              for(l = 0; l < blkn_per_component[k]; l++)
              {
                short lastDC = (short)scan->dc_diff[k];

                jerr = DecodeHuffman8x8(
                  state,
                  dc_huff_table[k],
                  ac_huff_table[k],
                  &lastDC,
                  MCUptr);

                scan->dc_diff[k] = lastDC;

                if(IJL_OK > jerr)
                {
                  TRACE0(trERROR,"ERROR: __g_decode_block_thumb() failed\n");
                  TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
                  return IJL_ERR_DATA;
                }
              }
            }

            jerr = Get_Restart(state, scan, scan_numxMCUs, scan_numyMCUs, i, j);

            if(IJL_OK != jerr)
            {
              TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
              TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
              return IJL_BAD_RST_MARKER;
            }
          } /* scan_numxMCUs */

          /* Store a copy of the current pointer in the rowoffset array. */
          Set_Pos(j+1, state, scan, jprops);
        } /* for topMCURow */
      }
      else /* if(jprops->rowoffsets[topMCURow].offset == 0xFFFFFFFF) */
      {
        jerr = Seek_Pos(topMCURow, state, scan, jprops);
        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: Seek_Pos() failed\n");
          TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
          return jerr;
        }
      }
    } /* if(scan->curxMCU == 0) */
  } /* if((jprops->jinterleaveType == 1) && (state->cur_scan_comp != 0)) */

  /*
  // We are now positioned at the top-left corner of the MCUroi.
  // We now decode (and process) a MCUroi width of MCUblocks.
  // Then decode blocks to wrap around to the left edge of the
  // MCUroi, and repeat.  Whenever we encounter a
  // start-of-MCUrow MCUblock, we update the value in rowoffsets.
  */

  for(j = topMCURow; j < MCURoi.bottom; j++)
  {
    /* 0. Decode blocks until we get to the */
    /* beginning of the MCUroi column. */
    MCUptr = (short*)jprops->MCUBuf;

    for(i = scan->curxMCU; i < MCURoi.left; i++)
    {
      for(k = 0; k < scan->ncomps; k++)
      {
        for(l = 0; l < blkn_per_component[k]; l++)
        {
          short lastDC = (short)scan->dc_diff[k];

          jerr = DecodeHuffman8x8(
            state,
            dc_huff_table[k],
            ac_huff_table[k],
            &lastDC,
            MCUptr);

          scan->dc_diff[k] = lastDC;

          if(IJL_OK > jerr)
          {
            TRACE0(trERROR,"ERROR: __g_decode_block_thumb() failed\n");
            TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
            return IJL_ERR_DATA;
          }
        }
      }

      jerr = Get_Restart(state, scan, scan_numxMCUs, scan_numyMCUs, i, j);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
        TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
        return IJL_BAD_RST_MARKER;
      }
    }

    if(!jprops->raw_coefs)
    {
      /* Set up the current position in the DIB. */
      state->DIB_ptr = (Ipp8u*)(jprops->DIBBytes + (rowGroup / scalefactor) * (j - MCURoi.top) + jprops->DIBChannels * (pixwidth/scalefactor) * max(scan->curxMCU - MCURoi.left, 0));
    }

    /* 1. Decode and process one MCURoi width. */
    for(i = max(scan->curxMCU,MCURoi.left); i < MCURoi.right; i++)
    {
      MCUptr = (short*)jprops->MCUBuf;

      for(k = 0; k < scan->ncomps; k++)
      {
        if(jprops->raw_coefs && !jprops->raw_coefs->data_type)
        {
          for(v = 0; v < scan->comps[k].vsampling; v++)
          {
            for(h = 0; h < scan->comps[k].hsampling; h++)
            {
              unsigned short* cvPtr;
              short lastDC = (short)scan->dc_diff[k];

              /* Decode + dequantize each block in the MCU. */
              jerr = DecodeHuffman8x8(
                state,
                dc_huff_table[k],
                ac_huff_table[k],
                &lastDC,
                MCUptr);

              scan->dc_diff[k] = lastDC;

              if(IJL_OK > jerr)
              {
                TRACE0(trERROR,"ERROR: __g_decode_block_thumb() failed\n");
                TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
                return IJL_ERR_DATA;
              }

              /*
              //                                [][]
              // rewrite block from [][][][] to [][] order
              */
              cvPtr = jprops->raw_coefs->raw_ptrs[k] +
                ((j * scan->comps[k].vsampling + v) * 8 * scan_numxMCUs * scan->comps[k].hsampling +
                 (i * scan->comps[k].hsampling + h))* 8;
              for(jj = 0; jj < 8; jj++, cvPtr += (scan_numxMCUs * scan->comps[k].hsampling * 8), MCUptr += 8)
              {
                IPPCALL(ippsCopy_8u)((const Ipp8u*)MCUptr,(Ipp8u*)cvPtr,sizeof(short) << 3);
              }
            } /* for h */
          } /* for v */
        } /* if raw coefs */
        else
        {
          for(l = 0; l < blkn_per_component[k]; l++)
          {
            short lastDC = (short)scan->dc_diff[k];

            /* Decode + dequantize each block in the MCU. */
            jerr = DecodeHuffman8x8(
              state,
              dc_huff_table[k],
              ac_huff_table[k],
              &lastDC,
              MCUptr);

            scan->dc_diff[k] = lastDC;

            iQnt8x8(MCUptr,quant_table[k]);

            if(IJL_OK > jerr)
            {
              TRACE0(trERROR,"ERROR: __g_decode_block_thumb() failed\n");
              TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
              return IJL_ERR_DATA;
            }

            MCUptr += DCTSIZE2;
          }
        }
      }

      MCUptr = (short*)jprops->MCUBuf;

      jerr = Get_Restart(state, scan, scan_numxMCUs, scan_numyMCUs, i, j);
      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
        TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
        return IJL_BAD_RST_MARKER;
      }

      if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
      {
        iDct_US_CC(
          jprops,
          state,
          scan,
          i,
          j,
          num8x8blks,
          (short*)jprops->MCUBuf,
          scan_numxMCUs,
          scan_numyMCUs,
          scalefactor,
          iDCT_instead_of_US);
      }

      /* Test for interrupts. */
      if(FALSE != jprops->interrupt)
      {
        if(jprops->jinterleaveType != 1)
        {
          if((m && (i==MCURoi.left)) || (n && (j==MCURoi.top)))
            continue;

          scan->curxMCU = (i + 1) % (MCURoi.right + m);

          if(i != (MCURoi.right + m - 1))
          {
            scan->curyMCU      = j;
            jprops->roi.right  = i-m;
            jprops->roi.bottom = j-n;
            jprops->roi.left   = i-m;
            jprops->roi.top    = j-n;

            TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");

            return IJL_INTERRUPT_OK;
          }
          else
          {
            scan->curxMCU = 0;
            scan->curyMCU = 0;
            goto skip_interrupt;
          }
        }
      } /* if interrupt */
skip_interrupt:
      ;
    }

    if(m)
    {
      iDct_US_CC(
        jprops,
        state,
        scan,
        i,
        j,
        num8x8blks,
        (short*)jprops->MCUBuf,
        scan_numyMCUs,
        scan_numyMCUs,
        scalefactor,
        iDCT_instead_of_US);

      /* Test for interrupts. */
      if(FALSE != jprops->interrupt)
      {
        if(jprops->jinterleaveType != 1)
        {
          scan->curxMCU = 0;
          scan->curyMCU = 0;
        }
      }
    }

    if((j + 1) < scan_numyMCUs)
    {
      /* If file is multi-scan. */
      if(jprops->jinterleaveType == 1 && state->cur_scan_comp != 0)
      {
        /* 3. Decode blocks until we get to the */
        /* beginning of the MCUroi column. */
        MCUptr = (short*)jprops->MCUBuf;

        for(i = MCURoi.right; i < scan_numxMCUs; i++)
        {
          for(k = 0; k < scan->ncomps; k++)
          {
            for(l = 0; l < blkn_per_component[k]; l++)
            {
              short lastDC = (short)scan->dc_diff[k];

              jerr = DecodeHuffman8x8(
                state,
                dc_huff_table[k],
                ac_huff_table[k],
                &lastDC,
                MCUptr);

              scan->dc_diff[k] = lastDC;

              if(IJL_OK > jerr)
              {
                TRACE0(trERROR,"ERROR: __g_decode_block_thumb() failed\n");
                TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
                return IJL_ERR_DATA;
              }
            }
          }

          jerr = Get_Restart(state, scan, scan_numxMCUs, scan_numyMCUs, i, j);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
            TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
            return IJL_BAD_RST_MARKER;
          }
        }
      }
      else
      {
        /* 2. Seek to the beginning of the next line. */
        if(jprops->rowoffsets[j+1].offset == 0xFFFFFFFF)
        {
          /* 3. Decode blocks until we get to the */
          /* beginning of the MCUroi column. */
          MCUptr = (short*)jprops->MCUBuf;

          for(i = MCURoi.right; i < scan_numxMCUs; i++)
          {
            for(k = 0; k < scan->ncomps; k++)
            {
              for(l = 0; l < blkn_per_component[k]; l++)
              {
                short lastDC = (short)scan->dc_diff[k];

                jerr = DecodeHuffman8x8(
                  state,
                  dc_huff_table[k],
                  ac_huff_table[k],
                  &lastDC,
                  MCUptr);

                scan->dc_diff[k] = lastDC;

                if(IJL_OK > jerr)
                {
                  return IJL_ERR_DATA;
                }
              }
            }

            jerr = Get_Restart(state, scan, scan_numxMCUs, scan_numyMCUs, i, j);

            if(IJL_OK != jerr)
            {
              TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
              TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
              return IJL_BAD_RST_MARKER;
            }
          }

          /* Store a copy of the current pointer in */
          /* the rowoffset array. */
          Set_Pos(j+1, state, scan, jprops);
        }
        else
        {
          jerr = Seek_Pos(j+1, state, scan, jprops);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: Seek_Pos() failed\n");
            TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");
            return jerr;
          }
        }
      }
    }
  }

  if(n)
  {
    for(i =  MCURoi.left; i < MCURoi.right + m; i++)
    {
      iDct_US_CC(
        jprops,
        state,
        scan,
        i,
        j,
        num8x8blks,
        (short*)jprops->MCUBuf,
        scan_numxMCUs,
        scan_numyMCUs,
        scalefactor,
        iDCT_instead_of_US);
    }
  }

  if((jprops->jframe.SeenAllACScans >= jprops->jframe.ncomps) || (jprops->jinterleaveType != 1))
  {
    jerr = IJL_ROI_OK;
  }
  else
  {
    jerr = IJL_OK;
  }

  TRACE0(trCALL|trAFTER,"leave DecodeScanBaselineRoi()\n");

  return jerr;
} /* DecodeScanBaselineRoi() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    DecodeScanProgressiveRoi
//
//  Purpose:
//
//  Context:
//
//  Returns:
//    Valid error code, or 0 for OK.
//
//  Parameters:
//    state       Pointer to IJL state variables.
//    scan        Pointer to current Scan variables.
//    jprops      Pointer to global IJL properties structure.
//    scalefactor Performs scaling (in the DCT's and color conversion).
//                Output data is scaled by 1/1, 1/2, or 1/4.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) DecodeScanProgressiveRoi(
  STATE*           state,
  SCAN*            scan,
  JPEG_PROPERTIES* jprops,
  int              scalefactor)
{
  long           i;
  int            j, k, l, m, jj, mm, nn;
  int            scan_numxMCUs, scan_numyMCUs;
  int            blkn_per_component[4];
  short*         MCUptr;
  short*         coef_buf;
  IJLERR         jerr;
  HUFFMAN_TABLE* ac_huff_table[4];
  HUFFMAN_TABLE* dc_huff_table[4];
  IJL_RECT       MCURoi;
  int            iDCT_instead_of_US;
  int num8x8blks = 0;
  int size       = 0;

  TRACE0(trCALL|trBEFORE,"enter in DecodeScanProgressiveRoi\n");

  /* Fill up temporary arrays with per-component information. */
  for(k = 0; k < scan->ncomps; k++)
  {
    if(!jprops->jscan)
    {
      scan->dc_diff[k] = 0;
    }

    ac_huff_table[k] = scan->comps[k].ac_table;
    dc_huff_table[k] = scan->comps[k].dc_table;

    blkn_per_component[k] = scan->comps[k].hsampling *
                            scan->comps[k].vsampling;

    num8x8blks += blkn_per_component[k];
  }

  if(scan->ncomps == 1)
  {
    num8x8blks            = 1;
    blkn_per_component[0] = 1;
  }

  if((jprops->jframe.max_hsampling == 1) && (jprops->jframe.max_vsampling == 1))
  {
    iDCT_instead_of_US = FALSE;
  }
  else
  {
    iDCT_instead_of_US = TRUE;

    for(i = 0; (i < jprops->jframe.ncomps) && iDCT_instead_of_US; i++)
    {
      if(jprops->jframe.comps[i].hsampling != jprops->jframe.comps[i].vsampling)
      {
        iDCT_instead_of_US = FALSE;
      }

      if((jprops->jframe.comps[i].hsampling != 1) && (jprops->jframe.comps[i].hsampling != 2))
      {
        iDCT_instead_of_US = FALSE;
      }
    }
  }

  MCURoi.left   = jprops->roi.left /
    ((8 / scalefactor) * jprops->jframe.max_hsampling);
  MCURoi.right  = (jprops->roi.right +
    ((8 / scalefactor) * jprops->jframe.max_hsampling) - 1) /
    ((8 / scalefactor) * jprops->jframe.max_hsampling);
  MCURoi.top    = jprops->roi.top /
    ((8 / scalefactor) * jprops->jframe.max_vsampling);
  MCURoi.bottom = (jprops->roi.bottom +
    ((8 / scalefactor) * jprops->jframe.max_vsampling) - 1) /
    ((8 / scalefactor) * jprops->jframe.max_vsampling);

  mm = ((jprops->upsampling_type == IJL_TRIANGLE_FILTER) && (jprops->jframe.max_hsampling == 2)) ? 1:0;

  nn = ((jprops->upsampling_type == IJL_TRIANGLE_FILTER) && (jprops->jframe.max_vsampling == 2)) ? 1:0;

  if(MCURoi.left)
    MCURoi.left -= mm;

  if(MCURoi.top)
    MCURoi.top  -= nn;

  if(MCURoi.right < jprops->numxMCUs)
    MCURoi.right  += mm;

  if(MCURoi.bottom < jprops->numyMCUs)
    MCURoi.bottom += nn;

  for(k = 0; k < jprops->jframe.ncomps; k++)
  {
    size += jprops->jframe.comps[k].hsampling *
            jprops->jframe.comps[k].vsampling;
  }

  /*
  // 1.  Skip MCU rows up to the MCURoi.top row.
  // 2.  Skip MCU columns up to the MCURoi.left row.
  // 3.  Process MCURoi.right - MCURoi.left MCUs.
  //     Output is sent to the DIB aligned on 0,0.
  // 4.  Skip MCU Columns up to the start of the next row.
  // 5.  Loop back to 2 if not processed
  //     MCURoi.bottom - MCURoi.top total MCURows.
  // 6.  Process rows to the end of the current scan.  Return.
  */

  if(scan->start_spec != 0 && scan->end_spec != 0)
  {
    /* AC coefficients. */

    i  = 8 * jprops->jframe.max_hsampling / scan->comps[0].hsampling;
    j  = 8 * jprops->jframe.max_vsampling / scan->comps[0].vsampling;

    scan_numxMCUs = (jprops->JPGWidth + i-1) / i;
    scan_numyMCUs = (abs(jprops->JPGHeight) + j-1) / j;

    for(j = scan->curyMCU; j < jprops->numyMCUs; j++)
    {
      /* AC Scan.  Only one coefficient at a time is processed. */
      if(jprops->jframe.SeenAllACScans <= jprops->jframe.ncomps)
      {
        for(jj = 0; jj < scan->comps[0].vsampling; jj++)
        {
          /*
          // Skip the last block row of the image if
          // greater than the image height.
          */
          if(((j * scan->comps[0].vsampling * 8) + (jj * 8)) >= jprops->JPGHeight)
            break;

          for(i = scan->curxMCU; i < jprops->numxMCUs; i++)
          {
            coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

            /* Skip over any relevant components. */
            for(k = 0; k < scan->comps[0].comp; k++)
            {
              coef_buf += (DCTSIZE2 * jprops->jframe.comps[k].hsampling *
                                      jprops->jframe.comps[k].vsampling);
            }

            /* Skip over relevant 8x8 blocks from this component. */
            coef_buf += (jj * DCTSIZE2 * scan->comps[0].hsampling);

            for(m = 0; m < scan->comps[0].hsampling; m++)
            {
              /* Ignore the last row(s) of the image. */
              if(((i * scan->comps[0].hsampling * 8) + (m * 8)) >= jprops->JPGWidth)
                break;

              if(scan->approx_high == 0)
              {
                /* AC Coefficient pass:  original. */
                Decode_Block_Prog_AC(
                  state,
                  coef_buf,
                  ac_huff_table[0],
                  scan->start_spec,
                  scan->end_spec,
                  scan->approx_low);

                coef_buf += DCTSIZE2;
              }
              else
              {
                Decode_Block_Prog_AC_SA(
                  state,
                  coef_buf,
                  ac_huff_table[0],
                  scan->start_spec,
                  scan->end_spec,
                  scan->approx_low);

                coef_buf += DCTSIZE2;
              }

              jerr = Get_Restart(state,scan,scan_numxMCUs,scan_numyMCUs,(i * scan->comps[0].hsampling) + m,(j * scan->comps[0].vsampling) + jj);

              if(IJL_OK != jerr)
              {
                TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
                TRACE0(trCALL|trAFTER,"leave DecodeScanProgressiveRoi()\n");
                return IJL_BAD_RST_MARKER;
              }
            } /* for(m = 0; m < scan->comps[0].hsampling; m++) */
          } /* for(i = scan->curxMCU; i < (int)jprops->numxMCUs; i++) */
        } /* for(jj = 0; jj < scan->comps[0].vsampling; jj++) */
      } /* if(jprops->jframe.SeenAllACScans <= jprops->jframe.ncomps) */

      /*
      // Process a whole MCUrow of data.
      // Following two lines are speed improvements when not
      // displaying per-scan information.
      // REMOVE THEM FOR VERSION 2.0.
      */
      if((jprops->jframe.SeenAllDCScans >= jprops->jframe.ncomps) &&
         (jprops->jframe.SeenAllACScans >= jprops->jframe.ncomps))
      {
        if((j >= MCURoi.top) && (j < MCURoi.bottom))
        {
          if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
          {
            /* Set up the current position in the DIB */
            state->DIB_ptr = jprops->DIBBytes +
              (8 / scalefactor) *
              (jprops->jframe.max_vsampling *
              (j - MCURoi.top) *
              jprops->DIBLineBytes + jprops->DIBChannels *
              jprops->jframe.max_hsampling *
              max((int)scan->curxMCU - MCURoi.left, 0));

            for(i = MCURoi.left; i < MCURoi.right; i++)
            {
              short* temp_mcu_ptr;
              coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

              MCUptr = (short*)jprops->MCUBuf;

              /* Copy stuff from coef_buf into MCUBuf. */
              IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)MCUptr,size*DCTSIZE2*sizeof(short));

              /* Dequantization AC coefficients */
              temp_mcu_ptr = MCUptr;
              for(m = 0; m < jprops->jframe.ncomps; m++)
              {
                int blks = jprops->jframe.comps[m].hsampling * jprops->jframe.comps[m].vsampling;
                for(l = 0; l < blks; l++)
                {
                  iQnt8x8(temp_mcu_ptr,jprops->jFmtQuant[jprops->jframe.comps[m].quant_sel].elements);
                  temp_mcu_ptr += DCTSIZE2;
                }
              }

              iDct_US_CC(
                jprops,
                state,
                scan,
                i,
                j,
                size,
                MCUptr,
                jprops->numxMCUs,
                jprops->numyMCUs,
                scalefactor,
                iDCT_instead_of_US);

            } /* for MCURoi.left ... MCURoi.right */

            if(mm && (i == MCURoi.right))
            {
              iDct_US_CC(
                jprops,
                state,
                scan,
                i,
                j,
                size,
                (short*)jprops->MCUBuf,
                jprops->numxMCUs,
                jprops->numyMCUs,
                scalefactor,
                iDCT_instead_of_US);
            }
          }
          else
          {
            /* only copy coefficients to user's buffer */
            for(i = MCURoi.left; i < MCURoi.right; i++)
            {
              coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

              MCUptr = (short*)jprops->MCUBuf;

              for(k = 0; k < jprops->jframe.ncomps; k++)
              {
                for(jj = 0; jj < jprops->jframe.comps[k].vsampling; jj++)
                {
                  for(m = 0; m < jprops->jframe.comps[k].hsampling; m++)
                  {
                    unsigned short* tmpPtr = jprops->raw_coefs->raw_ptrs[k] +
                    (j * jprops->numxMCUs * jprops->jframe.comps[k].hsampling *
                                            jprops->jframe.comps[k].vsampling * DCTSIZE2)  +
                    (jj * DCTSIZE2 * jprops->jframe.comps[k].hsampling * jprops->numxMCUs) +
                    (i * jprops->jframe.comps[k].hsampling * 8) + m * 8;

                    for(l = 0; l < 8; l++, tmpPtr += (jprops->numxMCUs * jprops->jframe.comps[k].hsampling * 8), coef_buf += 8)
                    {
                      IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)tmpPtr,sizeof(short) << 3);
                    }
                  } /* for m */
                } /* for jj */
              } /* for k */
            } /* for i */
          } /* !raw_coefficients */
        } /* if((j >= MCURoi.top) && (j < MCURoi.bottom)) */
      } /* if speedup */

      if(FALSE != jprops->interrupt)
      {
        scan->curyMCU = j + 1;
        scan->curxMCU = 0;

        if(scan->curyMCU == jprops->numyMCUs + nn)
        {
          scan->curyMCU = 0;

          TRACE0(trCALL|trAFTER,"leave DecodeScanProgressiveRoi()\n");

          return IJL_OK;
        }

        jprops->roi.right  = jprops->numxMCUs - 1;
        jprops->roi.bottom = j-nn;
        jprops->roi.left   = jprops->numxMCUs - 1;
        jprops->roi.top    = j-nn;

        TRACE0(trCALL|trAFTER,"leave DecodeScanProgressiveRoi()\n");

        return IJL_INTERRUPT_OK;
      } /* if interrupt */
    } /* for jprops->numyMCUs */

    if((!jprops->raw_coefs || jprops->raw_coefs->data_type) && nn)
    {
      for(i = MCURoi.left; i <= MCURoi.right; i++)
      {
        iDct_US_CC(
          jprops,
          state,
          scan,
          i,
          jprops->numyMCUs,
          size,
          (short*)jprops->MCUBuf,
          jprops->numxMCUs,
          jprops->numyMCUs,
          scalefactor,
          iDCT_instead_of_US);
      }

      if(FALSE != jprops->interrupt)
      {
        scan->curxMCU = 0;
        scan->curyMCU = 0;
        return IJL_OK;
      } /* if interrupt */
    }
  }
  else /* if((scan->start_spec != 0) && (scan->end_spec != 0)) */
  {
    /* DC Scan. */
    for(j = scan->curyMCU; j < jprops->numyMCUs; j++)
    {
      for(i = scan->curxMCU; i < jprops->numxMCUs; i++)
      {
        coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

        /* Skip over any relevant components. */
        for(k = 0; k < scan->comps[0].comp; k++)
        {
          coef_buf += (DCTSIZE2 * jprops->jframe.comps[k].hsampling *
                                  jprops->jframe.comps[k].vsampling);
        }

        MCUptr = (short*)jprops->MCUBuf;

        if(scan->approx_high == 0)
        {
          /* First DC scan.  No prior scans have been seen. */
          for(k = 0; k < scan->ncomps; k++)
          {
            for(l = 0; l < blkn_per_component[k]; l++)
            {
              short lastDC;
              /* Zero the coefficient buffer. */
              IPPCALL(ippsZero_8u)((Ipp8u*)coef_buf,DCTSIZE2*sizeof(short));

              lastDC = (short)scan->dc_diff[k];

              /* Decode each block in the MCU. */
              Decode_Block_Prog_DC(
                state,
                dc_huff_table[k],
                coef_buf,
                &lastDC,
                scan->approx_low);

              scan->dc_diff[k] = lastDC;

              coef_buf += DCTSIZE2;
            }
          }
        }
        else /* scan->approx_high == 0 */
        {
          for(k = 0; k < scan->ncomps; k++)
          {
            /* Successive approximation on the DC band. */
            for(l = 0; l < blkn_per_component[k]; l++)
            {
              /* Decode the DC coefficient. */
              Decode_Block_Prog_DC_SA(state,coef_buf,scan->approx_low);

              coef_buf += DCTSIZE2;
            }
          }
        } /* if scan->approx_high == 0 */

        jerr = Get_Restart(state, scan, jprops->numxMCUs, jprops->numyMCUs, i, j);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: Get_Restart() failed\n");
          TRACE0(trCALL|trAFTER,"leave Decode_Scan_Restart_Progressive_ROI()\n");
          return IJL_BAD_RST_MARKER;
        }

        if((i >= MCURoi.left) && (i < MCURoi.right)  &&
           (j >= MCURoi.top) &&  (j < MCURoi.bottom) &&
           (jprops->jframe.SeenAllDCScans >= jprops->jframe.ncomps) &&
           (jprops->jframe.SeenAllACScans >= jprops->jframe.ncomps))
        {

          coef_buf = jprops->coef_buffer + (DCTSIZE2 * size * (i + (j * jprops->numxMCUs)));

          if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
          {
            short* temp_mcu_ptr;
            MCUptr = (short*)jprops->MCUBuf;

            /* Copy stuff from coef_buf into MCUBuf. */
            IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)MCUptr,size*DCTSIZE2*sizeof(short));

            /* Dequantization DC coefficients */
            temp_mcu_ptr = (short*)jprops->MCUBuf;
            for(m = 0; m < jprops->jframe.ncomps; m++)
            {
              int blks = jprops->jframe.comps[m].hsampling * jprops->jframe.comps[m].vsampling;
              for(l = 0; l < blks; l++)
              {
                iQnt8x8(temp_mcu_ptr,jprops->jFmtQuant[jprops->jframe.comps[m].quant_sel].elements);
                temp_mcu_ptr += DCTSIZE2;
              }
            }

            iDct_US_CC(
              jprops,
              state,
              scan,
              i,
              j,
              size,
              MCUptr,
              jprops->numxMCUs,
              jprops->numyMCUs,
              scalefactor,
              iDCT_instead_of_US);
          }
          else
          {
            /* only copy to user's buffer */
            for(k = 0; k < jprops->jframe.ncomps; k++)
            {
              for(jj = 0; jj < jprops->jframe.comps[k].vsampling; jj++)
              {
                for(m = 0; m < jprops->jframe.comps[k].hsampling; m++)
                {
                  unsigned short* tmpPtr = jprops->raw_coefs->raw_ptrs[k] +
                  (j * jprops->numxMCUs * jprops->jframe.comps[k].hsampling *
                                          jprops->jframe.comps[k].vsampling * DCTSIZE2)  +
                  (jj * DCTSIZE2 * jprops->jframe.comps[k].hsampling * jprops->numxMCUs) +
                  (i * jprops->jframe.comps[k].hsampling * 8) + m * 8;
                  for(l = 0; l < 8; l++, tmpPtr += (jprops->numxMCUs * jprops->jframe.comps[k].hsampling * 8), coef_buf += 8)
                  {
                    IPPCALL(ippsCopy_8u)((const Ipp8u*)coef_buf,(Ipp8u*)tmpPtr,sizeof(short) << 3);
                  }
                } /* for m */
              } /* for jj */
            } /* for k */
          } /* if raw_coefs */
        } /* if ROI hit */

        /* process interrupt */
        if(FALSE != jprops->interrupt)
        {
          scan->curxMCU = (i + 1) % jprops->numxMCUs;

          if(scan->curxMCU != 0)
            scan->curyMCU = j;
          else
          {
            scan->curyMCU = (j + 1);
            if(scan->curyMCU == jprops->numyMCUs)
            {
              scan->curyMCU = 0;

              TRACE0(trCALL|trAFTER,"leave DecodeScanProgressiveRoi()\n");

              return IJL_OK;
            }
          }

          jprops->roi.right  = i-mm;
          jprops->roi.bottom = j-nn;
          jprops->roi.left   = i-mm;
          jprops->roi.top    = j-nn;

          TRACE0(trCALL|trAFTER,"leave DecodeScanProgressiveRoi()\n");

          return IJL_INTERRUPT_OK;
        } /* if interrupt */
      } /* for jprops->numxMCUs */
    } /* for jprops->numyMCUs */

  } /* if((scan->start_spec != 0) && (scan->end_spec != 0)) */

  if((jprops->jframe.SeenAllDCScans >= jprops->jframe.ncomps) &&
     (jprops->jframe.SeenAllACScans >= jprops->jframe.ncomps))
  {
    jerr = IJL_ROI_OK;
  }
  else
  {
    jerr = IJL_OK;
  }

  TRACE0(trCALL|trAFTER,"leave DecodeScanProgressiveRoi()\n");

  return jerr;
} /* DecodeScanProgressiveRoi() */



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Decode_Scan
//
//  Purpose
//    Scan Decoder Controller Module that decodes a
//    single scan from a frame.  The scan decoder is
//    responsible for detecting scans with restart (RST)
//    intervals and processing such scans differently
//    from "normal" (no restart) scans.
//
//  Context
//
//  Returns
//    Valid error code, or 0 for OK.
//
//  Parameters
//    *state    Pointer to IJL state variables.
//    *scan     Pointer to current scan variables.
//    *jprops   Pointer to global IJL properties structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Decode_Scan(
  IJL_MARKER*      marker,
  STATE*           state,
  SCAN*            scan,
  JPEG_PROPERTIES* jprops)
{
  int i;
  IJLERR jerr = IJL_OK;
  IppStatus status;

  *marker = MARKER_NONE;

  /* Test the ROI to see if the decode region is "inside" the image. */
  if((((jprops->roi.right < jprops->JPGWidth)  && (jprops->roi.right  != 0)) ||
     ((jprops->roi.bottom < jprops->JPGHeight) && (jprops->roi.bottom != 0)) ||
     (jprops->roi.top  > 0) ||
     (jprops->roi.left > 0)))
  {
    /* Decode a ROI within the image. */

    /* Clip the ROI to the image dimensions. */
    jprops->roi.right  = min(jprops->JPGWidth, jprops->roi.right);
    jprops->roi.bottom = min(jprops->JPGHeight, jprops->roi.bottom);
    jprops->roi.left   = max(0, jprops->roi.left);
    jprops->roi.top    = max(0, jprops->roi.top);

    /* If the rowoffsets array has not been allocated, do it now. */
    if(NULL == jprops->rowoffsets)
    {
      jprops->rowoffsets = (ENTROPYSTRUCT*)ippMalloc(sizeof(ENTROPYSTRUCT)*MAX_MCU_ROWS);

      if(NULL == jprops->rowoffsets)
      {
        return IJL_MEMORY_ERROR;
      }

      status = IPPCALL(ippiDecodeHuffmanStateInitAlloc_JPEG_8u)(
        &jprops->rowoffsets[0].pDecHuffState);

      if(ippStsNoErr != status)
      {
        return IJL_MEMORY_ERROR;
      }

      if((jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
         (jprops->iotype == IJL_JFILE_READENTROPY) ||
         (jprops->iotype == IJL_JFILE_READONEHALF) ||
         (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
         (jprops->iotype == IJL_JFILE_READONEEIGHTH))
      {
        jprops->rowoffsets[0].offset = state->entropy_bytes_processed;
      }
      else
      {
        jprops->rowoffsets[0].offset = (int)state->cur_entropy_ptr;
      }

      jprops->rowoffsets[0].dcval1        = 0;
      jprops->rowoffsets[0].dcval2        = 0;
      jprops->rowoffsets[0].dcval3        = 0;
      jprops->rowoffsets[0].dcval4        = 0;
      jprops->rowoffsets[0].unread_marker = state->unread_marker;

      for(i = 1; i < MAX_MCU_ROWS; i++)
      {
        status = IPPCALL(ippiDecodeHuffmanStateInitAlloc_JPEG_8u)(
          &jprops->rowoffsets[i].pDecHuffState);

        if(ippStsNoErr != status)
        {
          return IJL_MEMORY_ERROR;
        }

        jprops->rowoffsets[i].offset = 0xFFFFFFFF; /* uninitialized value */
      }
    }

    switch(jprops->iotype)
    {
      case IJL_JFILE_READONEEIGHTH:
      case IJL_JBUFF_READONEEIGHTH:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressiveRoi(state,scan,jprops,8);
        else
          jerr = DecodeScanBaselineRoi(state,scan,jprops,8);
        break;

      case IJL_JFILE_READONEQUARTER:
      case IJL_JBUFF_READONEQUARTER:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressiveRoi(state,scan,jprops,4);
        else
          jerr = DecodeScanBaselineRoi(state,scan,jprops,4);
        break;

      case IJL_JFILE_READONEHALF:
      case IJL_JBUFF_READONEHALF:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressiveRoi(state,scan,jprops,2);
        else
          jerr = DecodeScanBaselineRoi(state,scan,jprops,2);
        break;

      default:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressiveRoi(state,scan,jprops,1);
        else
          jerr = DecodeScanBaselineRoi(state,scan,jprops,1);
        break;

    } /* switch iotype */

    if(jprops->jinterleaveType)
    {
      if(NULL != jprops->rowoffsets)
      {
        for(i = 0; i < MAX_MCU_ROWS; i++)
        {
          status = IPPCALL(ippiDecodeHuffmanStateFree_JPEG_8u)(jprops->rowoffsets[i].pDecHuffState);

          if(ippStsNoErr != status)
          {
            return IJL_MEMORY_ERROR;
          }
        }
        ippFree(jprops->rowoffsets);
        jprops->rowoffsets = NULL;
      }
    }
  }
  else
  {
    /* Decode the entire image (ROI = entire image). */

    switch(jprops->iotype)
    {
      case IJL_JFILE_READONEEIGHTH:
      case IJL_JBUFF_READONEEIGHTH:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressive(state,scan,jprops,8);
        else
          jerr = DecodeScanBaseline(state,scan,jprops,8);
        break;

      case IJL_JFILE_READONEQUARTER:
      case IJL_JBUFF_READONEQUARTER:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressive(state,scan,jprops,4);
        else
          jerr = DecodeScanBaseline(state,scan,jprops,4);
        break;

      case IJL_JFILE_READONEHALF:
      case IJL_JBUFF_READONEHALF:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressive(state,scan,jprops,2);
        else
          jerr = DecodeScanBaseline(state,scan,jprops,2);
        break;

      default:

        if(jprops->progressive_found)
          jerr = DecodeScanProgressive(state,scan,jprops,1);
        else
          jerr = DecodeScanBaseline(state,scan,jprops,1);
        break;
    } /* switch iotype */
  } /* end roi */

  return jerr;
} /* Decode_Scan() */

