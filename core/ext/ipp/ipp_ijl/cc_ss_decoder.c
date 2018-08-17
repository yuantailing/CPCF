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
#ifndef __CC_SS_DECODER_H__
#include "cc_ss_decoder.h"
#endif


#ifdef __INTEL_COMPILER
#pragma warning(disable:279)
#endif



/* ///////////////////////////////////////////////////////////////////////////
// Macros/Constants
*/


/*
// The following routines implement the color conversion equations
// from CCIR Recommendation 601.
//
// In floating point notation, YCbCr->RGB:
//    R = Y                + (1.40200*Cr) - 179.45600
//    G = Y - (0.34414*Cb) - (0.71414*Cr) + 135.45984
//    B = Y + (1.77200*Cb)                - 226.81600
//
// In fixed point notation (using 8 bits), YCbCr->RGB:
//    R = ((256*Y) + (359*Cr) - 45941) / 256
//      = Y + (((359*Cr) - 45941) / 256)
//      = Y + (((359*Cr) - 45941 + 128) >> 8)
//      = Y + (((359*Cr) - 45813) >> 8)
//
//    G = ((256*Y) - (88*Cb) - (183*Cr) + 34678) / 256
//      = Y + (-(88*Cb) - (183*Cr) + 34678) / 256)
//      = Y + (-(88*Cb) - (183*Cr) + 34678 + 128) >> 8)
//      = Y + (-(88*Cb) - (183*Cr) + 34806) >> 8)
//
//    B = ((256*Y) + (454*Cb) - 58065) / 256
//      = Y + (((454*Cb) - 58065) / 256)
//      = Y + (((454*Cb) - 58065 + 128) >> 8)
//      = Y + (((454*Cb) - 57937) >> 8)
//
//    Note that a rounding technique for the right-shift operation is
//    incorporated directly into these fixed point equations
//    (i.e., a right-shift of N is preceded by adding 2^(N-1)).
//
//    Also note that these fixed point equations do *not* incorporate the
//    "level shift" of +128 that is needed after the inverse DCT.  This
//    is because the level shift has already been incorporated directly
//    into the iDCT routines.
//
*/

/*

IJL_INLINE int CC_YCBCR_R(int Y, int Cr)
{
  return CLIP( (Y + ( (  91881*Cr - 11760828) >> 16) ) );
}

IJL_INLINE int CC_YCBCR_G(int Y, int Cb, int Cr)
{
  return CLIP( (Y + ( ( -22554*Cb - 46802*Cr + 8877496) >> 16) ) );
}

IJL_INLINE int CC_YCBCR_B(int Y, int Cb)
{
  return CLIP( (Y + ( ( 116130*Cb - 14864613) >> 16) ) );
}

*/

#define CC_YCBCR_R(Y, Cr)			(CLIP( (Y + ( (  91881*Cr - 11760828) >> 16) ) ))
#define CC_YCBCR_G(Y, Cb, Cr)		(CLIP( (Y + ( ( -22554*Cb - 46802*Cr + 8877496) >> 16) ) ))
#define CC_YCBCR_B(Y, Cb)			(CLIP( (Y + ( ( 116130*Cb - 14864613) >> 16) ) ))

/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    CalcDIBRoi
//
//  Purpose
//    Determines the actual position to start writing data
//    into the DIB offset (x,y) into current MCU buffer to
//    get data from.
//
//  Context
//
//  Returns
//    Any IJLERR value.  IJL_OK indicates success.
//
//  Parameters
//    roi (in JPEG coordinates)
//    Current MCU x-coordinate
//    Current MCU y-coordinate
//    MCU Dimensions
//    Pointer to start of DIB, DIB line width (bytes)
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) CalcDIBRoi(
  const IJL_RECT* imageroi,
  const int       MCUxdim,
  const int       MCUydim,
  const int       curxMCU,
  const int       curyMCU,
  const int       DIBChannels,
  const int       DIBLineBytes,
  Ipp8u*          origDIB,
  const int       DIBWidth,
  const int       DIBHeight,
  Ipp8u**         newDIB,
  IJL_RECT*       DIBRect)
{
  /* Check to make sure this MCU has any parts within the ROI. */
  if(imageroi->left   == 0 &&
     imageroi->right  == 0 &&
     imageroi->top    == 0 &&
     imageroi->bottom == 0)
  {
    *newDIB = origDIB + (MCUxdim * curxMCU * DIBChannels) + (MCUydim * curyMCU * DIBLineBytes);

    DIBRect->left   = 0;
    DIBRect->right  = min(MCUxdim, DIBWidth - (MCUxdim * curxMCU));
    DIBRect->top    = 0;
    DIBRect->bottom = min(MCUydim, abs(DIBHeight) - (MCUydim * curyMCU));
  }
  else
  {
    if(((MCUxdim * curxMCU)     >= imageroi->right)  ||
       ((MCUxdim * (curxMCU+1)) <  imageroi->left)   ||
       ((MCUydim * curyMCU)     >= imageroi->bottom) ||
       ((MCUydim * (curyMCU+1)) <  imageroi->top))
    {
      return IJL_INVALID_JPEG_PROPERTIES;
    }

    *newDIB = origDIB + (((MCUxdim * curxMCU) - imageroi->left) * DIBChannels) + (((MCUydim * curyMCU) - imageroi->top) * DIBLineBytes);

    DIBRect->left   = max(0, (imageroi->left - MCUxdim * curxMCU));
    DIBRect->right  = min(min(MCUxdim, (imageroi->right - (MCUxdim * curxMCU))),(DIBWidth + imageroi->left - (MCUxdim * curxMCU)));
    DIBRect->top    = max(0, (imageroi->top - (MCUydim * curyMCU)));
    DIBRect->bottom = min(min(MCUydim, (imageroi->bottom - (MCUydim * curyMCU))),(abs(DIBHeight) + imageroi->top - (MCUydim * curyMCU)));
  }

  return IJL_OK;
} /* CalcDIBRoi() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Interleave_To_Output
//
//  Purpose
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) Interleave_To_Output(
  const JPEG_PROPERTIES* jprops,
  const IJL_RECT*        DIBRect,
  int                    i,
  Ipp8u*                 outptr,
  const Ipp8u*           tempptr)
{
  int j, k;

  if(i >= jprops->DIBChannels && jprops->DIBColor != IJL_YCBCR)
  {
    return;
  }

  if(jprops->JPGColor == IJL_G && jprops->DIBColor == IJL_RGBA_FPX)
  {
    /* Replicate first channel across all DIB channels. */
    for(j = DIBRect->top; j < DIBRect->bottom; j++)
    {
      for(k = DIBRect->left; k < DIBRect->right; k++)
      {
        outptr[(j * jprops->DIBLineBytes) + (k * 4) + 0] =
          tempptr[(j * 32) + k];
        outptr[(j * jprops->DIBLineBytes) + (k * 4) + 1] =
          tempptr[(j * 32) + k];
        outptr[(j * jprops->DIBLineBytes) + (k * 4) + 2] =
          tempptr[(j * 32) + k];
        outptr[(j * jprops->DIBLineBytes) + (k * 4) + 3] = (Ipp8u)IJL_OPAQUE;
      }
    }
  }
  else if((jprops->JPGColor == IJL_YCBCR || jprops->JPGColor == IJL_RGB) &&
          (jprops->DIBColor == IJL_RGBA_FPX))
  {
    if(i < 3)
    {
      for(j = DIBRect->top; j < DIBRect->bottom; j++)
      {
        for(k = DIBRect->left; k < DIBRect->right; k++)
        {
          outptr[(j * jprops->DIBLineBytes) + (k * 4)] = tempptr[(j * 32) + k];
        }
      }
    }
    else
    {
      for(j = DIBRect->top; j < DIBRect->bottom; j++)
      {
        for(k = DIBRect->left; k < DIBRect->right; k++)
        {
          outptr[(j * jprops->DIBLineBytes) + (k * 4)] = (Ipp8u)IJL_OPAQUE;
        }
      }
    }
  }
  else if((jprops->JPGColor == IJL_G) && (jprops->DIBColor == IJL_RGB) ||
          (jprops->JPGColor == IJL_G) && (jprops->DIBColor == IJL_BGR))
  {
    /* Replicate first channel across all DIB channels. */
    for(j = DIBRect->top; j < DIBRect->bottom; j++)
    {
      for(k = DIBRect->left; k < DIBRect->right; k++)
      {
        outptr[(j * jprops->DIBLineBytes) + (k * 3) + 0] =
          tempptr[(j * 32) + k];
        outptr[(j * jprops->DIBLineBytes) + (k * 3) + 1] =
          tempptr[(j * 32) + k];
        outptr[(j * jprops->DIBLineBytes) + (k * 3) + 2] =
          tempptr[(j * 32) + k];
      }
    }
  }
  else
  {
    for(j = DIBRect->top; j < DIBRect->bottom; j++)
    {
      for(k = DIBRect->left; k < DIBRect->right; k++)
      {
        outptr[(j * jprops->DIBLineBytes) + (k * jprops->DIBChannels)] =
          tempptr[(j * 32) + k];
      }
    }
  }

  return;
} /* Interleave_To_Output() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Output_Interleave_General_Scaled
//
//  Purpose
//    Copies data from the MCU buffer (jprops->MCUBuf) into the
//    output DIB, without color conversion or upsampling.
//
//    Additionally, the output get scaled by a specified factor.
//
//  Context
//    Called to process one MCU of data
//
//  Returns
//    none
//
//  jprops             Pointer to global IJL properties.
//  state              Pointer to current state of the IJL.
//  curxMCU             Used by an inline variant of SETUP_PTR (above)
//                      to indicate when the right edge of an image
//                      has been reached.
//  curyMCU             Used by an inline variant of SETUP_PTR (above)
//                      to indicate when the bottom edge of an image
//                      has been reached.
//  scaledim            Scaling factor.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) Output_Interleave_General_Scaled(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  int              curxMCU,
  int              curyMCU,
  int              scaledim)
{
  int      i, j, k, l, m;
  Ipp16s*  inptr;
  Ipp8u*   outptr;
  IJL_RECT DIBRect;
  IJLERR   jerr;


  inptr = (Ipp16s*)jprops->MCUBuf;

  jerr = CalcDIBRoi(
    &jprops->roi,
    scaledim,
    scaledim,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(state->DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
  {
    outptr = state->DIB_ptr + 2;
  }
  else
  {
    outptr = state->DIB_ptr;
  }

  for(i = 0; i < jprops->DIBChannels; i++)
  {
    if((i >= 1) && (jprops->JPGColor == IJL_G))
    {
      /*
      // Since the user wants to decode a grayscale image (1 channel)
      // into a >1-channel DIB, we need to fill in the "new" channels.
      // For example, if a 1-channel grayscale JPEG is decompressed
      // into a 3-byte buffer, the first byte will be correct and
      // the next two bytes will be set equal to the first channel.
      */

      for(l = DIBRect.top; l < DIBRect.bottom; l++)
      {
        for(m = DIBRect.left; m < DIBRect.right; m++)
        {
          if(i < 3)
          {
            outptr[(l * jprops->DIBLineBytes) + (m * jprops->DIBChannels)] =
            outptr[(l * jprops->DIBLineBytes) + (m * jprops->DIBChannels) - 1];
          }
          else /* i == 3 */
          {
            outptr[(l * jprops->DIBLineBytes) + (m * jprops->DIBChannels)] =
            (Ipp8u)CLIP(IJL_OPAQUE + 128);
          }
        }
      }
    }
    else if(i < jprops->jframe.ncomps)
    {
      for(j = 0; j < jprops->jframe.comps[i].vsampling; j++)
      {
        for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
        {
          for(l = DIBRect.top; l < DIBRect.bottom; l++)
          {
            for(m = DIBRect.left; m < DIBRect.right; m++)
            {
              outptr[(l * jprops->DIBLineBytes) + (m * jprops->DIBChannels)] =
                (Ipp8u)CLIP(inptr[(l * 8) + m] + 128);
            }
          }

          inptr += DCTSIZE2;
        }
      }
    }
    else if((i == 3) && (jprops->DIBColor == IJL_RGBA_FPX))
    {
      for(l = DIBRect.top; l < DIBRect.bottom; l++)
      {
        for(m = DIBRect.left; m < DIBRect.right; m++)
        {
          outptr[(l * jprops->DIBLineBytes) + (m * jprops->DIBChannels)] =
            (Ipp8u)CLIP(IJL_OPAQUE + 128);
        }
      }

      inptr += DCTSIZE2;
    }

    if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
    {
      outptr--;
    }
    else
    {
      outptr++;
    }
  }

  return;
} /* Output_Interleave_General_Scaled() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    US_General_Scaled
//
//  Purpose
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) US_General_Scaled(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  SCAN*            scan,
  int              curxMCU,
  int              curyMCU,
  int              scaledim,
  int              force_allch,
  int              iDCT_instead_of_US)
{
  int      i, j, k, l, m;
  int      h, v, h_expand, v_expand;
  int      pixwidth, pixheight;
  int      scan_maxhsampling, scan_maxvsampling;
  int      comp_i_vsampling, comp_i_hsampling;
  int      invalue;
  short*   inptr   = NULL;
  Ipp8u*   outptr  = NULL;
  Ipp8u*   tempptr = NULL;
  short*   leftBorderMCU;
  short*   rightBorderMCU;
  IJLERR   jerr;
  IJL_RECT DIBRect;
  Ipp8u*   temMCUBuf;
  Ipp8u    buf[MAX_MCU_SIZE*sizeof(short) + CPU_CACHE_LINE - 1];

  temMCUBuf = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

  if(scan->ncomps == 1) /* Non-interleaved data */
  {
    if(state->cur_scan_comp >= ((jprops->DIBColor == IJL_YCBCR) ? 3 : jprops->DIBChannels))
    {
      return;
    }

    scan_maxhsampling = jprops->jframe.max_hsampling /
      scan->comps[0].hsampling;
    scan_maxvsampling = jprops->jframe.max_vsampling /
      scan->comps[0].vsampling;

    pixwidth  = scaledim * scan_maxhsampling;
    pixheight = scaledim * scan_maxvsampling;
  }
  else
  {
    scan_maxhsampling = jprops->jframe.max_hsampling;
    scan_maxvsampling = jprops->jframe.max_vsampling;

    pixwidth  = scaledim * jprops->jframe.max_hsampling;
    pixheight = scaledim * jprops->jframe.max_vsampling;
  }

  /* Assume all upsampling is separable and assume MCUs are all */
  /* in the same scan (no progressive or hierarchical).         */

  if(jprops->upsampling_type == IJL_TRIANGLE_FILTER &&
     jprops->jframe.max_hsampling == 2 &&
     jprops->jframe.max_vsampling == 1)
  {
    if(jprops->jinterleaveType)
    {
      l = 1;
    }
    else
    {
      for(l = 0, i = jprops-> jframe.ncomps; i--;)
      {
        l += (jprops->jframe.comps[i].hsampling * jprops->jframe.comps[i].vsampling);
      }
    }

    l <<= 6;

    leftBorderMCU  = jprops->sampling_state_ptr->cur_row + (curxMCU+0) * l;
    rightBorderMCU = jprops->sampling_state_ptr->cur_row + (curxMCU+2) * l;
  }
  else
  {
    leftBorderMCU  = NULL;
    rightBorderMCU = NULL;
  }

  inptr = (Ipp16s*)jprops->MCUBuf;

  /*
  // Offset from the current (x,y) pixel location in the
  // output buffer is determined by the index of the
  // current scan component.
  */

  jerr = CalcDIBRoi(
    &jprops->roi,
    pixwidth,
    pixheight,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(state->DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  if(force_allch)
  {
    outptr = state->DIB_ptr;

    if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
    {
      if(state->cur_scan_comp == 0)
      {
        outptr += 2;
      }
      else if(state->cur_scan_comp == 1)
      {
        outptr += 1;
      }
      else
      {
        outptr = state->DIB_ptr;
      }
    }
  }
  else
  {
    if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
    {
      outptr = state->DIB_ptr;

      if(state->cur_scan_comp == 0)
      {
        outptr += 2;
      }
      else if(state->cur_scan_comp == 1)
      {
        outptr += 1;
      }
      else
      {
        outptr = state->DIB_ptr;
      }
    }
    else
    {
      outptr = state->DIB_ptr + state->cur_scan_comp;
    }
  }

  if(force_allch)
  {
    for(i = 0;
        i < ((jprops->DIBColor==IJL_YCBCR) ? 3 : min(scan->ncomps, jprops->DIBChannels));
        i++)
    {
      if((jprops->JPGColor == IJL_G) && (i >= 1))
      {
        continue;
      }

      if(scan->ncomps == 1)
      {
        comp_i_vsampling = scan->comps[0].vsampling;
        comp_i_hsampling = scan->comps[0].hsampling;
      }
      else
      {
        comp_i_vsampling = scan->comps[i].vsampling;
        comp_i_hsampling = scan->comps[i].hsampling;
      }

      h_expand = jprops->jframe.max_hsampling / comp_i_hsampling;
      v_expand = jprops->jframe.max_vsampling / comp_i_vsampling;

      if(!iDCT_instead_of_US)
      {
        /* Again, loop over the components in the scan, and not the frame. */
        if(jprops->upsampling_type == IJL_TRIANGLE_FILTER &&
           jprops->jframe.max_hsampling == 2 &&
           jprops->jframe.max_vsampling == 1 &&
           scaledim != 1)
        {
          int n = scaledim << 3;
          tempptr = temMCUBuf;

          for(k = 0; k < comp_i_hsampling; k++)
          {
            for(l = 0; l < n; l += 8)
            {
              int q = l << 2;

              if(h_expand == 2)
              {
                invalue = inptr[l] * 3; /* left column */
                tempptr[q + 0] = (Ipp8u)CLIP((invalue + leftBorderMCU[l + scaledim - 1] + 1 + 4*128) >> 2);
                tempptr[q + 1] = (Ipp8u)CLIP((invalue + inptr[l + 1] + 1 + 4*128) >> 2);
                for(m = 1; m < (scaledim-1); m++) /* middle columns */
                {
                  q += 2;
                  invalue = inptr[l + m] * 3;
                  tempptr[q + 0] = (Ipp8u)CLIP((invalue + inptr[l + m - 1] + 1 + 4*128) >> 2);
                  tempptr[q + 1] = (Ipp8u)CLIP((invalue + inptr[l + m + 1] + 2 + 4*128) >> 2);
                }

                invalue = inptr[ l + scaledim - 1] * 3; /* right column */
                tempptr[q + 2] = (Ipp8u)CLIP((invalue + inptr[l + scaledim - 2] + 2 + 4*128) >> 2);
                tempptr[q + 3] = (Ipp8u)CLIP((invalue + rightBorderMCU[l] + 2 + 4*128) >> 2);
              }
              else /* h_expand = 1 */
              {
                for(m = 0; m < scaledim; m++)
                {
                  tempptr[q + m ] = (Ipp8u)CLIP(inptr[l + m] + 128);
                }
              }

            }
            tempptr        += (scaledim * h_expand);
            inptr          += DCTSIZE2;
            rightBorderMCU += DCTSIZE2;
            leftBorderMCU  += DCTSIZE2;
          }
        }
        else
        {
          for(j = 0; j < comp_i_vsampling; j++)
          {
            tempptr = temMCUBuf + (scaledim * j * 32 * v_expand);

            for(k = 0; k < comp_i_hsampling; k++)
            {
              for(l = 0; l < scaledim; l++)
              {
                for(m = 0; m < scaledim; m++)
                {
                  for(v = 0; v < v_expand * 32; v += 32)
                  {
                    for(h = 0; h < h_expand; h++)
                    {
                      tempptr[v + h + (l * 32 * v_expand) + (m * h_expand)] =
                        (Ipp8u)CLIP(inptr[(l * 8) + m] + 128);
                    }
                  }
                }
              }
              tempptr += (scaledim * h_expand);
              inptr   += DCTSIZE2;
            }
          }
        }
      }
      else /* iDCT or US */
      {
        for(j = 0; j < comp_i_vsampling; j++)
        {
          tempptr = temMCUBuf + (scaledim * j * 32 * v_expand);
          for(k = 0; k < comp_i_hsampling; k++)
          {
            for(l = 0; l < scaledim * v_expand; l++)
            {
              for(m = 0; m < scaledim * h_expand; m++)
              {
                tempptr[(l * 32) + m] = (Ipp8u)CLIP(inptr[(l * 8) + m] + 128);
              }
            }
            tempptr += (scaledim * h_expand);
            inptr   += DCTSIZE2;
          }
        }
      }

      tempptr = temMCUBuf;

      /* Convert temporary buffer to output buffer. */
      if((jprops->DIBColor == IJL_YCBCR) && (i != 0))
      {
        if(i == 2)
        {
          outptr++;
        }

        for(j = DIBRect.top; j < DIBRect.bottom; j++)
        {
          for(k = DIBRect.left; k < DIBRect.right; k += 2)
          {
            outptr[(j * jprops->DIBLineBytes) + (k * 2)] =
              tempptr[(j * 32) + k];
          }
        }
      }
      else
      {
        Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);
      }

      /* Increment to the next channel. */
      if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
      {
        outptr--;
      }
      else
      {
        outptr++;
      }
    }

    if(jprops->DIBColor != IJL_OTHER || jprops->JPGColor != IJL_OTHER)
    {
      for(; i < jprops->DIBChannels; i++)
      {
        /* Convert temporary buffer to output buffer. */
        Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);

        /* Increment to the next channel. */
        if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
        {
          outptr--;
        }
        else
        {
          outptr++;
        }
      }
    }
  }
  else
  {
    for(i = 0; i < min(scan->ncomps, jprops->DIBChannels); i++)
    {
      if((jprops->JPGColor == IJL_G) && (i >= 1))
      {
        continue;
      }

      if(scan->ncomps == 1)
      {
        comp_i_vsampling = scan->comps[0].vsampling;
        comp_i_hsampling = scan->comps[0].hsampling;
      }
      else
      {
        comp_i_vsampling = scan->comps[i].vsampling;
        comp_i_hsampling = scan->comps[i].hsampling;
      }

      h_expand = jprops->jframe.max_hsampling / comp_i_hsampling;
      v_expand = jprops->jframe.max_vsampling / comp_i_vsampling;

      if(!iDCT_instead_of_US)
      {
        /* Again, loop over the components in the scan, and not the frame. */
        if(jprops->upsampling_type == IJL_TRIANGLE_FILTER &&
           jprops->jframe.max_hsampling == 2 &&
           jprops->jframe.max_vsampling == 1 &&
           scaledim != 1)
        {
          int n = scaledim << 3;
          tempptr = temMCUBuf;

          for(k = 0; k < comp_i_hsampling; k++)
          {
            for(l = 0; l < n; l += 8)
            {
              int q = l << 2;

              if(h_expand == 2)
              {
                invalue = inptr[l] * 3; /* left column */
                tempptr[q + 0] = (Ipp8u)CLIP((invalue + leftBorderMCU[l + scaledim - 1] + 1 + 4*128) >> 2);
                tempptr[q + 1] = (Ipp8u)CLIP((invalue + inptr[l + 1] + 2 + 4*128) >> 2);
                for(m = 1; m < (scaledim-1); m++) /* middle columns */
                {
                  q += 2;
                  invalue = inptr[l + m] * 3;
                  tempptr[q + 0] = (Ipp8u)CLIP((invalue + inptr[l + m - 1] + 1 + 4*128) >> 2);
                  tempptr[q + 1] = (Ipp8u)CLIP((invalue + inptr[l + m + 1] + 2 + 4*128) >> 2);
                }

                invalue = inptr[l + scaledim - 1] * 3; /* right column */
                tempptr[q + 2] = (Ipp8u)CLIP((invalue + inptr[l + scaledim - 2] + 1 + 4*128) >> 2);
                tempptr[q + 3] = (Ipp8u)CLIP((invalue + rightBorderMCU[l] + 2 + 4*128) >> 2);
              }
              else /* h_expand = 1 */
              {
                for(m = 0; m < scaledim; m++)
                {
                  tempptr[q + m ] = (Ipp8u)CLIP(inptr[l + m] + 128);
                }
              }

            }
            tempptr        += (scaledim * h_expand);
            inptr          += DCTSIZE2;
            rightBorderMCU += DCTSIZE2;
            leftBorderMCU  += DCTSIZE2;
          }
        }
        else /* any upSampling */
        {
          for(j = 0; j < comp_i_vsampling; j++)
          {
            tempptr = temMCUBuf + (scaledim * j * 32 * v_expand);

            for(k = 0; k < comp_i_hsampling; k++)
            {
              for(l = 0; l < scaledim; l++)
              {
                for(m = 0; m < scaledim; m++)
                {
                  for(v = 0; v < (v_expand * 32); v += 32)
                  {
                    for(h = 0; h < h_expand; h++)
                    {
                      tempptr[v + h + (l * 32 * v_expand) + (m * h_expand)] =
                        (Ipp8u)CLIP(inptr[(l * 8) + m] + 128);
                    }
                  }
                }
              }
              tempptr += (scaledim * h_expand);
              inptr   += DCTSIZE2;
            }
          }
        }
      }
      else /* idct instead of upsampling */
      {
        for(j = 0; j < comp_i_vsampling; j++)
        {
          tempptr = temMCUBuf + (scaledim * j * 32 * v_expand);
          for(k = 0; k < comp_i_hsampling; k++)
          {
            for(l = 0; l < scaledim * v_expand; l++)
            {
              for(m = 0; m < scaledim * h_expand; m++)
              {
                tempptr[(l * 32) + m] = (Ipp8u)CLIP(inptr[(l * 8) + m] + 128);
              }
            }
            tempptr += (scaledim * h_expand);
            inptr   += DCTSIZE2;
          }
        }
      }

      tempptr = temMCUBuf;

      /* Convert temporary buffer to output buffer. */
      if(jprops->DIBColor == IJL_YCBCR && state->cur_scan_comp != 0)
      {
        if(state->cur_scan_comp == 2)
        {
          outptr++;
        }

        for(j = DIBRect.top; j < DIBRect.bottom; j++)
        {
          for(k = DIBRect.left; k < DIBRect.right; k += 2)
          {
            outptr[(j * jprops->DIBLineBytes) + (k * 2)] =
              tempptr[(j * 32) + k];
          }
        }
      }
      else
      {
        Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);
      }

      if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
      {
        outptr--;
      }
      else
      {
        outptr++;
      }
    }

    if(jprops->jinterleaveType)
    {
      i = (state->cur_scan_comp == (jprops->JPGChannels-1)) ? jprops->JPGChannels :
                                                              jprops->DIBChannels;
    }

    if(jprops->DIBColor != IJL_OTHER || jprops->JPGColor != IJL_OTHER)
    {
      for(; i < jprops->DIBChannels; i++)
      {
        /* Convert temporary buffer to output buffer. */
        Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);

        /* Increment to the next channel. */
        if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
        {
          outptr--;
        }
        else
        {
          outptr++;
        }
      }
    }
  } /* !force_allch */

  return;
} /* US_General_Scaled() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    US_General_Scaled_P
//
//  Purpose
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) US_General_Scaled_P(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  int              curxMCU,
  int              curyMCU,
  int              scaledim,
  int              iDCT_instead_of_US)
{
  int      i, j, k, l, m;
  int      h, v, h_expand, v_expand;
  int      invalue;
  short*   leftBorderMCU;
  short*   rightBorderMCU;
  short*   inptr   = NULL;
  Ipp8u*   outptr  = NULL;
  Ipp8u*   tempptr = NULL;
  IJLERR   jerr;
  IJL_RECT DIBRect;
  Ipp8u*   temMCUBuf;
  Ipp8u    buf[MAX_MCU_SIZE*sizeof(short) + CPU_CACHE_LINE - 1];

  temMCUBuf = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

  /* Assume all upsampling is separable and assume MCUs are all */
  /* in the same scan (no progressive or hierarchical).         */

  if(jprops->upsampling_type == IJL_TRIANGLE_FILTER &&
     jprops->jframe.max_hsampling == 2 &&
     jprops->jframe.max_vsampling == 1)
  {
    for(l = 0, i = jprops->jframe.ncomps; i--;)
    {
      l += jprops->jframe.comps[i].hsampling *
           jprops->jframe.comps[i].vsampling;
    }

    l <<= 6;

    leftBorderMCU  = jprops->sampling_state_ptr->cur_row + (curxMCU+0) * l;
    rightBorderMCU = jprops->sampling_state_ptr->cur_row + (curxMCU+2) * l;
  }
  else
  {
    leftBorderMCU  = NULL;
    rightBorderMCU = NULL;
  }

  inptr = (short*)jprops->MCUBuf;

  /*
  // Offset from the current (x,y) pixel location in the
  // output buffer is determined by the index of the
  // current scan component.
  */

  jerr = CalcDIBRoi(
    &jprops->roi,
    scaledim*jprops->jframe.max_hsampling,
    scaledim*jprops->jframe.max_vsampling,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(state->DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  outptr = state->DIB_ptr;

  if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
  {
    if(state->cur_scan_comp == 0)
    {
      outptr += 2;
    }
    else if(state->cur_scan_comp == 1)
    {
      outptr += 1;
    }
    else
    {
      outptr = state->DIB_ptr;
    }
  }

  for(i = 0;
      i < min(jprops->jframe.ncomps, ((jprops->DIBColor == IJL_YCBCR) ? 3 : jprops->DIBChannels));
      i++)
  {
    if((jprops->JPGColor == IJL_G) && (i >= 1))
    {
      continue;
    }

    h_expand = jprops->jframe.max_hsampling / jprops->jframe.comps[i].hsampling;
    v_expand = jprops->jframe.max_vsampling / jprops->jframe.comps[i].vsampling;

    if(!iDCT_instead_of_US)
    {
      /* Again, loop over the components in the scan, and not the frame. */
      if(jprops->upsampling_type == IJL_TRIANGLE_FILTER &&
         jprops->jframe.max_hsampling == 2 &&
         jprops->jframe.max_vsampling == 1 &&
         scaledim != 1)
      {
        int n = scaledim << 3;
        tempptr = temMCUBuf;

        for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
        {
          for(l = 0; l < n; l += 8)
          {
            int q = l << 2;

            if(h_expand == 2)
            {
              invalue = inptr[l] * 3; /* left column */
              tempptr[q + 0] = (Ipp8u)CLIP((invalue + leftBorderMCU[l + scaledim - 1] + 1 + 4*128) >> 2);
              tempptr[q + 1] = (Ipp8u)CLIP((invalue + inptr[l + 1] + 2 + 4*128) >> 2);
              for(m = 1; m < (scaledim-1); m++) /* middle columns */
              {
                q += 2;
                invalue = inptr[l + m] * 3;
                tempptr[q + 0] = (Ipp8u)CLIP((invalue + inptr[l + m - 1] + 1 + 4*128) >> 2);
                tempptr[q + 1] = (Ipp8u)CLIP((invalue + inptr[l + m + 1] + 2 + 4*128) >> 2);
              }

              invalue = inptr[l + scaledim - 1] * 3; /* right column */
              tempptr[q + 2] = (Ipp8u)CLIP((invalue + inptr[l + scaledim - 2] + 1 + 4*128) >> 2);
              tempptr[q + 3] = (Ipp8u)CLIP((invalue + rightBorderMCU[l] + 2 + 4*128) >> 2);
            }
            else /* h_expand = 1 */
            {
              for(m = 0; m < scaledim; m++)
              {
                tempptr[q + m] = (Ipp8u)CLIP(inptr[l + m] + 128);
              }
            }

          }
          tempptr        += (scaledim * h_expand);
          inptr          += DCTSIZE2;
          rightBorderMCU += DCTSIZE2;
          leftBorderMCU  += DCTSIZE2;
        }
      }
      else
      {
        for(j = 0; j < jprops->jframe.comps[i].vsampling; j++)
        {
          tempptr = temMCUBuf + (scaledim * j * 32 * v_expand);

          for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
          {
            for(l = 0; l < scaledim; l++)
            {
              for(m = 0; m < scaledim; m++)
              {
                for(v = 0; v < (v_expand * 32); v += 32)
                {
                  for(h = 0; h < h_expand; h++)
                  {
                    tempptr[v + h + (l * 32 * v_expand) + (m * h_expand)] =
                      (Ipp8u)CLIP(inptr[(l * 8) + m] + 128);
                  }
                }
              }
            }
            tempptr += (scaledim * h_expand);
            inptr   += DCTSIZE2;
          }
        }
      }
    }
    else /* iDCT or US */
    {
      for(j = 0; j < jprops->jframe.comps[i].vsampling; j++)
      {
        tempptr = temMCUBuf + (scaledim * j * 32 * v_expand);
        for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
        {
          for(l = 0; l < scaledim * v_expand; l++)
          {
            for(m = 0; m < scaledim * h_expand; m++)
            {
              tempptr[(l * 32) + m] = (Ipp8u)CLIP(inptr[(l * 8) + m] + 128);
            }
          }
          tempptr += (scaledim * h_expand);
          inptr   += DCTSIZE2;
        }
      }
    }

    tempptr = temMCUBuf;

    /* Convert temporary buffer to output buffer. */
    if((jprops->DIBColor == IJL_YCBCR) && i)
    {
      if(i == 2)
      {
        outptr++;
      }

      for(j = DIBRect.top; j < DIBRect.bottom; j++)
      {
        for(k = DIBRect.left; k < DIBRect.right; k += 2)
        {
          outptr[(j * jprops->DIBLineBytes) + (k * 2)] =
            tempptr[(j * 32) + k];
        }
      }
    }
    else
    {
      Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);
    }

    /* Increment to the next channel. */
    if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
    {
      outptr--;
    }
    else
    {
      outptr++;
    }
  }

  if(jprops->jinterleaveType)
  {
    i = (state->cur_scan_comp == (jprops->JPGChannels-1)) ? jprops->JPGChannels :
                                                            jprops->DIBChannels;
  }
  if(jprops->DIBColor != IJL_OTHER || jprops->JPGColor != IJL_OTHER)
  {
    for(; i < jprops->DIBChannels; i++)
    {
      /* Convert temporary buffer to output buffer. */
      Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);

      /* Increment to the next channel. */
      if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
      {
        outptr--;
      }
      else
      {
        outptr++;
      }
    }
  }

  return;
} /* US_General_Scaled_P() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    CC_YCbCr_InPlace_Scaled
//
//  Purpose
//    Always called after an interleaving routine.
//    Thus the channels will be correct except possibly for
//    color twists.
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) CC_YCbCr_InPlace_Scaled(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  int              curxMCU,
  int              curyMCU,
  int              scaledim)
{
  int      i, j;
  Ipp8u*   outptr;
  Ipp16s   Y, Cb, Cr;
  Ipp16s   r, g, b;
  IJL_RECT DIBRect;
  IJLERR   jerr;

  /* Don't do color conversion if the DIB type is inappropriate. */
  if(jprops->DIBColor != IJL_RGB &&
     jprops->DIBColor != IJL_BGR &&
     jprops->DIBColor != IJL_RGBA_FPX)
  {
    return;
  }

  jerr = CalcDIBRoi(
    &jprops->roi,
    scaledim * jprops->jframe.max_hsampling,
    scaledim * jprops->jframe.max_vsampling,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(state->DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  outptr = state->DIB_ptr;

  if(jprops->DIBChannels == 3)
  {
    for(j = DIBRect.top; j < DIBRect.bottom; j++)
    {
      for(i = (DIBRect.left * 3); i < (DIBRect.right * 3); i += 3)
      {
        Y  = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 0];
        Cb = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 1];
        Cr = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 2];

        r = (Ipp16s)CC_YCBCR_R(Y, Cr);
        g = (Ipp16s)CC_YCBCR_G(Y, Cb, Cr);
        b = (Ipp16s)CC_YCBCR_B(Y, Cb);

        if(jprops->DIBColor == IJL_BGR)
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)b;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)r;
        }
        else
        {
          /* jprops->DIBColor == IJL_RGB ||   */
          /* jprops->DIBColor == IJL_RGBA_FPX */
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)r;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)b;
        }
      }
    }
  }
  else /* DIBChannels >= 4 */
  {
    /* jprops->DIBColor == IJL_RGBA_FPX */
    for(j = DIBRect.top; j < DIBRect.bottom; j++)
    {
      for(i = (DIBRect.left * jprops->DIBChannels); i < (DIBRect.right * jprops->DIBChannels); i += jprops->DIBChannels)
      {
        Y  = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 0];
        Cb = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 1];
        Cr = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 2];

        r = (Ipp16s)CC_YCBCR_R(Y, Cr);
        g = (Ipp16s)CC_YCBCR_G(Y, Cb, Cr);
        b = (Ipp16s)CC_YCBCR_B(Y, Cb);

        if(jprops->JPGColor == IJL_YCBCRA_FPX)
        {
          /* Do the FlashPix "flip" where X = 255 - X'. */
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)(255 - r);
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)(255 - g);
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)(255 - b);
        }
        else if(jprops->JPGColor == IJL_YCBCR)
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)r;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)b;
        }
      }
    }
  }

  return;
} /* CC_YCbCr_InPlace_Scaled() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    US_CC_General_YCbCr_Scaled
//
//  Purpose
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) US_CC_General_YCbCr_Scaled(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  SCAN*            scan,
  int              curxMCU,
  int              curyMCU,
  int              scaledim,
  int              iDCT_instead_of_US)
{
  /* Call the general upsampling algorithm. */
  if(scan->start_spec == 0 && jprops->progressive_found == 0)
  {
    US_General_Scaled(
      jprops,
      state,
      scan,
      curxMCU,
      curyMCU,
      scaledim,
      TRUE,
      iDCT_instead_of_US);
  }
  else
  {
    US_General_Scaled_P(
      jprops,
      state,
      curxMCU,
      curyMCU,
      scaledim,
      iDCT_instead_of_US);
  }

  /* Call the color conversion algorithm. */
  CC_YCbCr_InPlace_Scaled(jprops, state, curxMCU, curyMCU, scaledim);

  return;
} /* US_CC_General_YCbCr_Scaled() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    CC_General_YCbCr_Scaled
//
//  Purpose
//
//  Context
//
//  Returns
//    none
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) CC_General_YCbCr_Scaled(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  int              curxMCU,
  int              curyMCU,
  int              scaledim)
{
  /* Call the general non-upsampled interleave-to-output routine. */
  Output_Interleave_General_Scaled(
    jprops,
    state,
    curxMCU,
    curyMCU,
    scaledim);

  /* Call the color conversion algorithm. */
  CC_YCbCr_InPlace_Scaled(
    jprops,
    state,
    curxMCU,
    curyMCU,
    scaledim);

  return;
} /* CC_General_YCbCr_Scaled() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    US_And_CC_MCU_Scaled
//
//  Purpose
//    This routine controls which color conversion and/or
//    upsampling path is applied to a single MCU of data on
//    scaled decoding.
//    Conversion is either through highly optimized conversion
//    paths or through general-purpose functions.
//    General conversions support pixel or plane interleaved
//    data (see function descriptions below).
//
//  Context
//    Called for each MCU to be decoded.
//
//  Returns
//    none
//
//  Parameters
//    jprops     Pointer to global IJL properties structure.
//    state      Pointer to current operating state of the IJL.
//    scan       Pointer to scan of the current MCU.
//    curxMCU     The current MCU horizontal index, represented as an
//                integer from 0-(jprops->numxMCUs-1), inclusive.
//    curyMCU     The current MCU vertical index, represented as an
//                integer from 0-(jprops->numyMCUs-1), inclusive.
//    scaledim
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) US_And_CC_MCU_Scaled(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  SCAN*            scan,
  int              curxMCU,
  int              curyMCU,
  int              scaledim,
  int              iDCT_instead_of_US)
{
  if(!jprops->jinterleaveType || jprops->progressive_found)
  {
    if(jprops->fast_processing)
    {
      goto slow_upsample_cc;
    }
    else
    {
      /* ////////////////////////////////////////////////////////////////////
      // Slower "generic" unoptimized modes.
      */

slow_upsample_cc:

      if(jprops->upsampling_reqd && jprops->cconversion_reqd)
      {
        /* Only currently supported color space is YCbCr. */
        US_CC_General_YCbCr_Scaled(
          jprops,
          state,
          scan,
          curxMCU,
          curyMCU,
          scaledim,
          iDCT_instead_of_US);
      }
      else if(!jprops->upsampling_reqd && jprops->cconversion_reqd)
      {
        /* Only supported color space is YCbCr. */
        CC_General_YCbCr_Scaled(
          jprops,
          state,
          curxMCU,
          curyMCU,
          scaledim);
      }
      else if(jprops->upsampling_reqd && !jprops->cconversion_reqd)
      {
        /* General upsampling algorithm. */
        if(scan->start_spec == 0 && !jprops->progressive_found)
        {
          US_General_Scaled(
            jprops,
            state,
            scan,
            curxMCU,
            curyMCU,
            scaledim,
            TRUE,
            iDCT_instead_of_US);
        }
        else
        {
          US_General_Scaled_P(
            jprops,
            state,
            curxMCU,
            curyMCU,
            scaledim,
            iDCT_instead_of_US);
        }
      }
      else /* No upsampling or color conversion required. */
      {
        if(jprops->DIBColor == IJL_YCBCR)
        {
          /* Only currently supported color space is YCbCr. */
          US_CC_General_YCbCr_Scaled(
            jprops,
            state,
            scan,
            curxMCU,
            curyMCU,
            scaledim,
            iDCT_instead_of_US);
        }
        else
        {
          Output_Interleave_General_Scaled(
            jprops,
            state,
            curxMCU,
            curyMCU,
            scaledim);
        }
      }
    }
  }
  else
  {
    /* Non-interleaved color conversion. */
    US_General_Scaled(
      jprops,
      state,
      scan,
      curxMCU,
      curyMCU,
      scaledim,
      FALSE,
      iDCT_instead_of_US);
  }

  return;
} /* US_And_CC_MCU_Scaled() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    CConvert_Image
//
//  Purpose
//    Used in the multi-scan (or plane-interleaved) approach to
//    encoding JPEG images.  Writes data to the output DIB
//    from the MCU buffer with enough padding between pixels to
//    allow subsequent scans to be written.  Thus this returns a
//    pixel interleaved image from a scan-interleaved image.
//
//    Needs to fill in opacity channel when decoding to
//    RGBA_FPX image from YCbCr data.
//
//  Context
//    Called only in baseline mode and only if the image
//    has multiple scans.
//
//  Returns
//    none
//
//  Parameters
//    jprops  Pointer to global IJL properties structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) CConvert_Image(
  JPEG_PROPERTIES* jprops)
{
  int    i, j, k;
  int    Y, Cb, Cr;
  Ipp8u  r, g, b;
  Ipp8u* outptr = jprops->DIBBytes;

  if(jprops->DIBColor != IJL_BGR &&
     jprops->DIBColor != IJL_RGB &&
     jprops->DIBColor != IJL_RGBA_FPX)
  {
    return;
  }

  if(jprops->cconversion_reqd == FALSE)
  {
    return;
  }

  j = jprops->roi.bottom - jprops->roi.top;

  if(j == 0)
  {
    j = abs(jprops->DIBHeight);
  }

  k = (jprops->roi.right - jprops->roi.left) * jprops->DIBChannels;

  if(k == 0)
  {
    k = jprops->DIBWidth * jprops->DIBChannels;
  }

  if(jprops->DIBChannels == 3)
  {
    for(; j--; )
    {
      for(i = 0; i < k; i += jprops->DIBChannels)
      {
        Y  = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 0];
        Cb = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 1];
        Cr = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 2];

        r = (Ipp8u)CC_YCBCR_R(Y, Cr);
        g = (Ipp8u)CC_YCBCR_G(Y, Cb, Cr);
        b = (Ipp8u)CC_YCBCR_B(Y, Cb);

        if(jprops->DIBColor == IJL_BGR)
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)b;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)r;
        }
        else /* (jprops->DIBColor == IJL_RGB) */
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)r;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)b;
        }
      }
    }
  }
  else
  {
    /* jprops->DIBColor == IJL_RGBA_FPX */
    for(; j--; )
    {
      for(i = 0; i < k; i += jprops->DIBChannels)
      {
        Y  = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 0];
        Cb = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 1];
        Cr = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 2];

        r = (Ipp8u)CC_YCBCR_R(Y, Cr);
        g = (Ipp8u)CC_YCBCR_G(Y, Cb, Cr);
        b = (Ipp8u)CC_YCBCR_B(Y, Cb);

        if(jprops->JPGColor == IJL_YCBCRA_FPX)
        {
          /* Do the FlashPix "flip" where X = 255 - X'. */
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)(255 - r);
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)(255 - g);
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)(255 - b);
        }
        else if(jprops->JPGColor == IJL_YCBCR)
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)r;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)b;
          outptr[i + (j * jprops->DIBLineBytes) + 3] = (Ipp8u)IJL_OPAQUE;
        }
      }
    }
  }

  return;
} /* CConvert_Image() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    US_General_P_MCU
//
//  Purpose
//    Performs general upsampling on an MCU of data.
//    Upsampling is "box" upsampling (i.e., data is
//    replicated n times, where n is the upsampling
//    factor).  Works for any JPEG-compliant upsampling
//    factors (up to 4:1 across four channels).
//    Data is assumed pixel interleaved.
//
//  Context
//    Called to process one MCU of data
//
//  Returns
//    none
//
//  Parameters
//    jprops              Pointer to global IJL properties.
//    state               Pointer to current state of the IJL.
//    curxMCU              Used by an inline variant of SETUP_PTR (above)
//                         to indicate when the right edge of an image
//                         has been reached.
//    curyMCU              Used by an inline variant of SETUP_PTR (above)
//                         to indicate when the bottom edge of an image
//                         has been reached.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) US_General_P_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int      i, j, k, l, m, q;
  int      h, v, h_expand, v_expand;
  int      invalue, rvalue, tvalue, lvalue, bvalue;
  int      s, ltvalue, rtvalue, lbvalue, rbvalue, sampType;
  Ipp16s   *leftBorderMCU, *rightBorderMCU, *topBorderMCU, *bottomBorderMCU;
  Ipp16s   *ltBorderMCU, *rtBorderMCU, *lbBorderMCU, *rbBorderMCU;
  Ipp16s*  inptr   = NULL;
  Ipp8u*   outptr  = NULL;
  Ipp8u*   tempptr = NULL;
  IJLERR   jerr;
  IJL_RECT DIBRect;
  Ipp8u*   temMCUBuf;
  Ipp8u    buf[MAX_MCU_SIZE * sizeof(Ipp16s) + CPU_CACHE_LINE - 1];

  temMCUBuf = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

  leftBorderMCU   = NULL;
  rightBorderMCU  = NULL;
  topBorderMCU    = NULL;
  bottomBorderMCU = NULL;
  ltBorderMCU     = NULL;
  rtBorderMCU     = NULL;
  lbBorderMCU     = NULL;
  rbBorderMCU     = NULL;

  sampType = 0;

  if(IJL_TRIANGLE_FILTER == jprops->upsampling_type)
  {
    if(jprops->jframe.max_hsampling == 2 &&
       jprops->jframe.max_vsampling == 1)
    {
      sampType = 1;
      for(k = jprops->jframe.ncomps, l = 0; k--;)
      {
        l += jprops->jframe.comps[k].hsampling *
             jprops->jframe.comps[k].vsampling;
      }

      l <<= 6;

      leftBorderMCU  = jprops->sampling_state_ptr->cur_row +  curxMCU    * l;
      rightBorderMCU = jprops->sampling_state_ptr->cur_row + (curxMCU+2) * l;
    }
    if(jprops->jframe.max_hsampling == 2 &&
       jprops->jframe.max_vsampling == 2)
    {
      sampType = 2;
      for(k = jprops->jframe.ncomps, l = 0; k--;)
      {
        l += jprops->jframe.comps[k].hsampling *
             jprops->jframe.comps[k].vsampling;
      }

      l <<= 6;

      j = (jprops->numxMCUs + 2);
      m = (curxMCU + 1) * l;
      s = (curxMCU + 2) * l;

      topBorderMCU    = jprops->sampling_state_ptr->top_row + m;
      ltBorderMCU     = jprops->sampling_state_ptr->top_row + curxMCU * l;
      rtBorderMCU     = jprops->sampling_state_ptr->top_row + s;

      leftBorderMCU   = jprops->sampling_state_ptr->cur_row + curxMCU * l;
      rightBorderMCU  = jprops->sampling_state_ptr->cur_row + s;

      bottomBorderMCU = jprops->sampling_state_ptr->bottom_row + m;
      lbBorderMCU     = jprops->sampling_state_ptr->bottom_row + curxMCU * l;
      rbBorderMCU     = jprops->sampling_state_ptr->bottom_row + s;
    }
  }

  inptr = (Ipp16s*)jprops->MCUBuf;

  jerr = CalcDIBRoi(
    &jprops->roi,
    jprops->jframe.max_hsampling * 8,
    jprops->jframe.max_vsampling * 8,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(jprops->state.DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  /* Offset from the current (x,y) pixel location in the output buffer */
  /* is determined by the index of the current scan component.         */

  if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
  {
    outptr = jprops->state.DIB_ptr;

    if(jprops->state.cur_scan_comp == 0)
    {
      outptr += 2;
    }
    else if(jprops->state.cur_scan_comp == 1)
    {
      outptr += 1;
    }
  }
  else
  {
    outptr = jprops->state.DIB_ptr + jprops->state.cur_scan_comp;
  }

  /* Loop over the components in the scan. */
  for(i = 0; i < min(jprops->jframe.ncomps, jprops->DIBChannels); i++)
  {
    if((jprops->JPGColor == IJL_G) && (i >= 1))
    {
      continue;
    }

    h_expand = jprops->jframe.max_hsampling / jprops->jframe.comps[i].hsampling;
    v_expand = jprops->jframe.max_vsampling / jprops->jframe.comps[i].vsampling;

    if(sampType == 1) /* as 422 */
    {
      /* Again, we loop over the components in the scan, and not the frame. */
      tempptr = temMCUBuf;
      for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
      {
        for(l = 0; l < 256; l += 32)
        {

          if(h_expand == 2)
          {
            invalue = inptr[0]*3;
            tempptr[l + 0] = (Ipp8u)CLIP((invalue + leftBorderMCU[7] + 1 + 4*128) >> 2);
            tempptr[l + 1] = (Ipp8u)CLIP((invalue + inptr[1] + 2 + 4*128) >> 2);

            for(q = 1; q < 7; q++)
            {
              m = q << 1;
              invalue = inptr[q]*3;
              tempptr[l + m + 0] = (Ipp8u)CLIP((invalue + inptr[q-1] + 1 + 4*128) >> 2);
              tempptr[l + m + 1] = (Ipp8u)CLIP((invalue + inptr[q+1] + 2 + 4*128) >> 2);
            }

            invalue = inptr[7] * 3;
            tempptr[l + 14] = (Ipp8u)CLIP((invalue + inptr[6] + 1 + 4*128) >> 2);
            tempptr[l + 15] = (Ipp8u)CLIP((invalue + rightBorderMCU[0] + 2 + 4*128) >> 2);
          }
          else
          {
            for(m = 0; m < 8; m ++)
            {
              tempptr[l + m] = (Ipp8u)CLIP(inptr[m] + 128);
            }
          }
          inptr          += 8;
          rightBorderMCU += 8;
          leftBorderMCU  += 8;
        }
        tempptr += 8 * h_expand;
      }
    }
    else if(sampType == 2) /* as 411 */
    {
      /* Again, loop over the components in the scan, and not the frame. */
      for(j = 0; j < jprops->jframe.comps[i].vsampling; j++)
      {
        tempptr = temMCUBuf + (j * 8 * 32 * v_expand);

        for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
        {
          if(h_expand == 2 && v_expand == 2)
          {
            for(l = 0, s = 8; s--; l += 64)
            {
              invalue = inptr[0] * 9;
              lvalue  = leftBorderMCU[7] * 3;
              rvalue  = inptr[1] * 3;

              if(l != 0)
              {
                tvalue   = inptr[-8] * 3;
                ltvalue  = leftBorderMCU[-1];
                rtvalue  = inptr[-7];
              }
              else
              {
                tvalue  = topBorderMCU[56] * 3;
                ltvalue = ltBorderMCU[63];
                rtvalue = topBorderMCU[57];
              }
              if(s != 0)
              {
                bvalue  = inptr[8] * 3;
                lbvalue = leftBorderMCU[15];
                rbvalue = inptr[9];
              }
              else
              {
                bvalue  = bottomBorderMCU[0] * 3;
                lbvalue = lbBorderMCU[7];
                rbvalue = bottomBorderMCU[1];
              }

              tempptr[l +  0] = (Ipp8u)CLIP((invalue + lvalue + tvalue + ltvalue + 8 + 16*128) >> 4);
              tempptr[l +  1] = (Ipp8u)CLIP((invalue + rvalue + tvalue + rtvalue + 7 + 16*128) >> 4);
              tempptr[l + 32] = (Ipp8u)CLIP((invalue + lvalue + bvalue + lbvalue + 8 + 16*128) >> 4);
              tempptr[l + 33] = (Ipp8u)CLIP((invalue + rvalue + bvalue + rbvalue + 7 + 16*128) >> 4);

              inptr++;
              topBorderMCU++;
              bottomBorderMCU++;

              for(m = 2; m < 14; m += 2)
              {
                /* 3/4 * nearer pixel + 1/4 * further pixel in each */
                /* dimension, thus 9/16, 3/16, 3/16, 1/16 overall   */

                invalue = inptr[0] * 9;
                lvalue  = inptr[-1] * 3;
                rvalue  = inptr[1] * 3;

                if(l != 0)
                {
                  tvalue  = inptr[-8] * 3;
                  ltvalue = inptr[-9];
                  rtvalue = inptr[-7];
                }
                else
                {
                  tvalue  = topBorderMCU[56] * 3;
                  ltvalue = topBorderMCU[55];
                  rtvalue = topBorderMCU[57];
                }
                if(s != 0)
                {
                  bvalue  = inptr[8] * 3;
                  lbvalue = inptr[7];
                  rbvalue = inptr[9];
                }
                else
                {
                  bvalue  = bottomBorderMCU[0] * 3;
                  lbvalue = bottomBorderMCU[-1];
                  rbvalue = bottomBorderMCU[1];
                }

                tempptr[l + m +  0] = (Ipp8u)CLIP((invalue + lvalue + tvalue + ltvalue + 8 + 16*128) >> 4);
                tempptr[l + m +  1] = (Ipp8u)CLIP((invalue + rvalue + tvalue + rtvalue + 7 + 16*128) >> 4);
                tempptr[l + m + 32] = (Ipp8u)CLIP((invalue + lvalue + bvalue + lbvalue + 8 + 16*128) >> 4);
                tempptr[l + m + 33] = (Ipp8u)CLIP((invalue + rvalue + bvalue + rbvalue + 7 + 16*128) >> 4);

                inptr++;
                topBorderMCU++;
                bottomBorderMCU++;
              }

              invalue = inptr[0] * 9;
              lvalue  = inptr[-1] * 3;
              rvalue  = rightBorderMCU[0] * 3;
              if(l != 0)
              {
                tvalue  = inptr[-8] * 3;
                ltvalue = inptr[-9];
                rtvalue = rightBorderMCU[-8];
              }
              else
              {
                tvalue  = topBorderMCU[56] * 3;
                ltvalue = topBorderMCU[55];
                rtvalue = rtBorderMCU[56];
              }
              if(s != 0)
              {
                bvalue  = inptr[8] * 3;
                lbvalue = inptr[7];
                rbvalue = rightBorderMCU[8];
              }
              else
              {
                bvalue  = bottomBorderMCU[0] * 3;
                lbvalue = bottomBorderMCU[-1];
                rbvalue = rbBorderMCU[0];
              }

              tempptr[l + 14] = (Ipp8u)CLIP((invalue + lvalue + tvalue + ltvalue + 8 + 16*128) >> 4);
              tempptr[l + 15] = (Ipp8u)CLIP((invalue + rvalue + tvalue + rtvalue + 7 + 16*128) >> 4);
              tempptr[l + 46] = (Ipp8u)CLIP((invalue + lvalue + bvalue + lbvalue + 8 + 16*128) >> 4);
              tempptr[l + 47] = (Ipp8u)CLIP((invalue + rvalue + bvalue + rbvalue + 7 + 16*128) >> 4);

              inptr++;
              topBorderMCU++;
              bottomBorderMCU++;

              rightBorderMCU  += 8;
              leftBorderMCU   += 8;
              topBorderMCU    -= 8;
              bottomBorderMCU -= 8;
            }

            tempptr         += 16;
            topBorderMCU    += DCTSIZE2;
            bottomBorderMCU += DCTSIZE2;
            ltBorderMCU     += DCTSIZE2;
            rtBorderMCU     += DCTSIZE2;
            lbBorderMCU     += DCTSIZE2;
            rbBorderMCU     += DCTSIZE2;
          }
          else
          {
            /* Again, we loop over the components in the scan, and not the frame. */
            for(j = 0; j < jprops->jframe.comps[i].vsampling; j++)
            {
              tempptr = temMCUBuf + (j * 8 * 32 * v_expand);

              for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
              {
                for(l = 0; l < (8 * v_expand * 32); l += (v_expand * 32))
                {
                  for(m = 0; m < (8 * h_expand); m += h_expand)
                  {
                    /* The v and h loops are correct.  No need to modify. */
                    invalue = *inptr++;

                    for(v = 0; v < (v_expand * 32); v += 32)
                    {
                      for(h = 0; h < h_expand; h++)
                      {
                        tempptr[l + m + v + h] = (Ipp8u)CLIP(invalue + 128);
                      }
                    }
                  }
                }
                tempptr         += (8 * h_expand);
                topBorderMCU    += DCTSIZE2;
                leftBorderMCU   += DCTSIZE2;
                rightBorderMCU  += DCTSIZE2;
                bottomBorderMCU += DCTSIZE2;
                ltBorderMCU     += DCTSIZE2;
                rtBorderMCU     += DCTSIZE2;
                lbBorderMCU     += DCTSIZE2;
                rbBorderMCU     += DCTSIZE2;
              }
            }
          }
        }
      }
    }
    else /* 411 */
    {
      /* Again, we loop over the components in the scan, and not the frame. */
      for(j = 0; j < jprops->jframe.comps[i].vsampling; j++)
      {
        tempptr = temMCUBuf + (j * 8 * 32 * v_expand);

        for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
        {
          for(l = 0; l < (8 * v_expand * 32); l += (v_expand * 32))
          {
            for(m = 0; m < (8 * h_expand); m += h_expand)
            {
              /* The v and h loops are correct.  No need to modify. */
              invalue = *inptr++;

              for(v = 0; v < (v_expand * 32); v += 32)
              {
                for(h = 0; h < h_expand; h++)
                {
                  tempptr[l + m + v + h] = (Ipp8u)CLIP(invalue + 128);
                }
              }
            }
          }
          tempptr += (8 * h_expand);
        }
      }
    }

    tempptr = temMCUBuf;

    /* Convert temporary buffer to output buffer. */
    Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);

    /* Increment to the next channel. */
    if((jprops->DIBColor == IJL_BGR) && (jprops->JPGColor == IJL_RGB))
    {
      outptr--;
    }
    else
    {
      outptr++;
    }
  }

  if(jprops->jinterleaveType)
  {
    i = (jprops->state.cur_scan_comp == (jprops->JPGChannels-1)) ? jprops->JPGChannels :
                                                                   jprops->DIBChannels;
  }

  if(jprops->DIBColor != IJL_OTHER || jprops->JPGColor != IJL_OTHER)
  {
    for(; i < jprops->DIBChannels; i++)
    {
      /* Convert temporary buffer to output buffer. */
      Interleave_To_Output(jprops, &DIBRect, i, outptr, tempptr);

      /* Increment to the next channel. */
      if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
      {
        outptr--;
      }
      else
      {
        outptr++;
      }
    }
  }

  return;
} /* US_General_P_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    CC_YCbCr_InPlace
//
//  Purpose
//    Performs YCbCr->RGB color conversion in-place on 1 MCU of
//    image data in the output DIB.  The color conversion
//    equation is derived from the JFIF specification.
//    Supports variable DIB channel-per-pixel values
//    and line widths.
//
//  Context
//    Called to process one MCU of data
//
//  Returns
//    none
//
//  Parameters
//    jprops   Pointer to global IJL properties.
//    state    Pointer to current state of the IJL.
//    curxMCU  Used by an inline variant of SETUP_PTR (above)
//             to indicate when the right edge of an image
//             has been reached.
//    curyMCU  Used by an inline variant of SETUP_PTR (above)
//             to indicate when the bottom edge of an image
//             has been reached.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(void) CC_YCbCr_InPlace(
  JPEG_PROPERTIES* jprops,
  STATE*           state,
  int              curxMCU,
  int              curyMCU)
{
  int      i, j;
  Ipp16s   Y, Cb, Cr;
  Ipp16s   r, g, b;
  Ipp8u*   outptr;
  IJLERR   jerr;
  IJL_RECT DIBRect;


  /* Don't do the color conversion if the color format */
  /* is not IJL_BGR, IJL_RGB, or IJL_RGBA_FPX.         */
  if(jprops->DIBColor != IJL_BGR &&
     jprops->DIBColor != IJL_RGB &&
     jprops->DIBColor != IJL_RGBA_FPX)
  {
    return;
  }

  jerr = CalcDIBRoi(
    &jprops->roi,
    8 * jprops->jframe.max_hsampling,
    8 * jprops->jframe.max_vsampling,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(state->DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  outptr = state->DIB_ptr;

  if(jprops->DIBChannels == 3)
  {
    for(j = DIBRect.top; j < DIBRect.bottom; j++)
    {
      for(i = (DIBRect.left * 3); i < (DIBRect.right * 3); i += 3)
      {
        Y  = (Ipp16s)(outptr[i + (j * jprops->DIBLineBytes) + 0]);
        Cb = (Ipp16s)(outptr[i + (j * jprops->DIBLineBytes) + 1]);
        Cr = (Ipp16s)(outptr[i + (j * jprops->DIBLineBytes) + 2]);

        r = (Ipp16s)CC_YCBCR_R(Y, Cr);
        g = (Ipp16s)CC_YCBCR_G(Y, Cb, Cr);
        b = (Ipp16s)CC_YCBCR_B(Y, Cb);

        if(jprops->DIBColor == IJL_BGR)
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)b;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)r;
        }
        else /* jprops->DIBColor == IJL_RGB */
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)r;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)b;
        }
      }
    }
  }
  else /* DIBChannels >= 4 */
  {
    /* jprops->DIBColor == IJL_RGBA_FPX */
    for(j = DIBRect.top; j < DIBRect.bottom; j++)
    {
      for(i = (DIBRect.left * jprops->DIBChannels); i < (DIBRect.right * jprops->DIBChannels); i += jprops->DIBChannels)
      {
        Y  = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 0];
        Cb = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 1];
        Cr = (Ipp16s)outptr[i + (j * jprops->DIBLineBytes) + 2];

        r = (Ipp16s)CC_YCBCR_R(Y, Cr);
        g = (Ipp16s)CC_YCBCR_G(Y, Cb, Cr);
        b = (Ipp16s)CC_YCBCR_B(Y, Cb);

        if(jprops->JPGColor == IJL_YCBCRA_FPX)
        {
          /* Do the FlashPix "flip" where X = 255 - X'. */
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)(255 - r);
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)(255 - g);
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)(255 - b);
        }
        else if(jprops->JPGColor == IJL_YCBCR)
        {
          outptr[i + (j * jprops->DIBLineBytes) + 0] = (Ipp8u)r;
          outptr[i + (j * jprops->DIBLineBytes) + 1] = (Ipp8u)g;
          outptr[i + (j * jprops->DIBLineBytes) + 2] = (Ipp8u)b;
        }
      }
    }
  }

  return;
} /* CC_YCbCr_InPlace() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_111_to_RGB_MCU
//
//  Purpose
//    Performs YCbCr->RGB color conversion, and output data
//    formatting of DCT output blocks, for a 1:1:1 MCU.
//
//    This routine is a C implementation.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_111_to_RGB_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  Ipp16s*  inptr = (Ipp16s*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[8*8*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*1]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*2]; /* Cr */

    IPPCALL(ippiYCbCr444ToRGBLS_MCU_16s8u_P3C3R)(pSrc,pDst,8*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,8*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,8*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* YCbCr_111_to_RGB_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_111_to_BGR_MCU
//
//  Purpose
//    Performs YCbCr->BGR color conversion, and output data
//    formatting of DCT output blocks, for a 1:1:1 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_111_to_BGR_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[8*8*3 + CPU_CACHE_LINE-1];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    const Ipp16s* pSrc[3];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*1]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*2]; /* Cr */

    IPPCALL(ippiYCbCr444ToBGRLS_MCU_16s8u_P3C3R)(pSrc,pDst,8*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,8*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,8*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* YCbCr_111_to_BGR_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_422_to_RGB_MCU
//
//  Purpose
//    Performs upsampling and YCbCr -> RGR color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:2:2 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_422_to_RGB_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*8*3 + CPU_CACHE_LINE-1];

    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*2]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*3]; /* Cr */

    IPPCALL(ippiYCbCr422ToRGBLS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,16*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,16*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* YCbCr_422_to_RGB_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_422_to_BGR_MCU
//
//  Purpose
//    Performs upsampling and YCbCr->BGR color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:2:2 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_422_to_BGR_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*8*3 + CPU_CACHE_LINE-1];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    const Ipp16s* pSrc[3];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*2]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*3]; /* Cr */

    IPPCALL(ippiYCbCr422ToBGRLS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,16*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,16*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* YCbCr_422_to_BGR_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_411_to_RGB_MCU
//
//  Purpose
//    Performs upsampling and YCbCr->RGB color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:1:1 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_411_to_RGB_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJLERR   jerr;
  IJL_RECT dibrect;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          16,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*16*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*4]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*5]; /* Cr */

    IPPCALL(ippiYCbCr411ToRGBLS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,16*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,16*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* YCbCr_411_to_RGB_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_411_to_BGR_MCU
//
//  Purpose
//    Performs upsampling and YCbCr->BGR color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:1:1 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_411_to_BGR_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          16,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*16*3 + CPU_CACHE_LINE-1];
    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);
    const Ipp16s* pSrc[3];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*4]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*5]; /* Cr */

    IPPCALL(ippiYCbCr411ToBGRLS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,16*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,16*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* YCbCr_411_to_BGR_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_111_to_RGBA_FPX_MCU
//
//  Purpose
//    Performs YCbCr->RGBA_FPX color conversion and
//    output data formatting of DCT output blocks
//    for a subset of a 1:1:1 MCU.
//    The alpha channel gets seeded with an opaque value.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_111_to_RGBA_FPX_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[8*8*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*1]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*2]; /* Cr */

    IPPCALL(ippiYCbCr444ToRGBLS_MCU_16s8u_P3C3R)(pSrc,pDst,8*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;
      IPPCALL(ippiMirror_8u_C3IR)(pDst,8*3,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;
    }

    IPPCALL(ippiCopy_8u_C3AC4R)(pDst,8*3,outptr,lineoffset,roi);
    IPPCALL(ippiSet_8u_C4CR)((Ipp8u)IJL_OPAQUE,outptr+3,lineoffset,roi);
  }

  return;
} /* YCbCr_111_to_RGBA_FPX_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_422_to_RGBA_FPX_MCU
//
//  Purpose
//    Performs upsampling and YCbCr->RGBA_FPX color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:2:2 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_422_to_RGBA_FPX_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*8*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*2]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*3]; /* Cr */

    IPPCALL(ippiYCbCr422ToRGBLS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;
      IPPCALL(ippiMirror_8u_C3IR)(pDst,16*3,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;
    }

    IPPCALL(ippiCopy_8u_C3AC4R)(pDst,16*3,outptr,lineoffset,roi);
    IPPCALL(ippiSet_8u_C4CR)((Ipp8u)IJL_OPAQUE,outptr+3,lineoffset,roi);
  }

  return;
} /* YCbCr_422_to_RGBA_FPX_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_411_to_RGBA_FPX_MCU
//
//  Purpose
//    Performs upsampling and YCbCr->RGBA_FPX color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:1:1 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_411_to_RGBA_FPX_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          16,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*16*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrc[1] = &inptr[DCTSIZE2*4]; /* Cb */
    pSrc[2] = &inptr[DCTSIZE2*5]; /* Cr */

    IPPCALL(ippiYCbCr411ToRGBLS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;
      IPPCALL(ippiMirror_8u_C3IR)(pDst,16*3,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;
    }

    IPPCALL(ippiCopy_8u_C3AC4R)(pDst,16*3,outptr,lineoffset,roi);
    IPPCALL(ippiSet_8u_C4CR)((Ipp8u)IJL_OPAQUE,outptr+3,lineoffset,roi);
  }

  return;
} /* YCbCr_411_to_RGBA_FPX_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCrA_FPX_1111_to_RGBA_FPX_MCU
//
//  Purpose
//    Performs YCbCrA_FPX->RGBA_FPX color conversion and
//    output data formatting of DCT output blocks for a
//    subset of a 1:1:1:1 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCrA_FPX_1111_to_RGBA_FPX_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[8*8*4 + CPU_CACHE_LINE-1];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    const Ipp16s* pSrcMCU[4];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    pSrcMCU[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrcMCU[1] = &inptr[DCTSIZE2*1]; /* Cb */
    pSrcMCU[2] = &inptr[DCTSIZE2*2]; /* Cr */
    pSrcMCU[3] = &inptr[DCTSIZE2*3]; /* A  */

    IPPCALL(ippiYCCK444ToCMYKLS_MCU_16s8u_P4C4R)(pSrcMCU,pDst,8*4);

    outptr = jprops->state.DIB_ptr;

    if(lineoffset < 0)
    {
      IppiSize c1roi;
      c1roi.width  = dibrect.right*4;
      c1roi.height = dibrect.bottom;

      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      /* to cover absent functionality of ippiMirror_8u_C4IR */
      IPPCALL(ippiMirror_8u_C1IR)(pDst,8*4,c1roi,ippAxsHorizontal);
    }

    IPPCALL(ippiCopy_8u_C4R)(pDst,8*4,outptr,lineoffset,roi);
  }

  return;
}  /* YCbCrA_FPX_1111_to_RGBA_FPX_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCrA_FPX_4224_to_RGBA_FPX_MCU
//
//  Purpose
//    Performs upsampling and YCbCrA_FPX->RGBA_FPX color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:2:2:4 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCrA_FPX_4224_to_RGBA_FPX_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*8*4 + CPU_CACHE_LINE-1];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    const Ipp16s* pSrcMCU[4];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrcMCU[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrcMCU[1] = &inptr[DCTSIZE2*2]; /* Cb */
    pSrcMCU[2] = &inptr[DCTSIZE2*3]; /* Cr */
    pSrcMCU[3] = &inptr[DCTSIZE2*4]; /* A  */

    IPPCALL(ippiYCCK422ToCMYKLS_MCU_16s8u_P4C4R)(pSrcMCU,pDst,16*4);

    outptr = jprops->state.DIB_ptr;

    if(lineoffset < 0)
    {
      IppiSize c1roi;
      c1roi.width  = dibrect.right*4;
      c1roi.height = dibrect.bottom;

      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      /* to cover absent functionality of ippiMirror_8u_C4IR */
      IPPCALL(ippiMirror_8u_C1IR)(pDst,16*4,c1roi,ippAxsHorizontal);
    }

    IPPCALL(ippiCopy_8u_C4R)(pDst,16*4,outptr,lineoffset,roi);
  }

  return;
}  /* YCbCrA_FPX_4224_to_RGBA_FPX_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCrA_FPX_4114_to_RGBA_FPX_MCU
//
//  Purpose
//    Performs upsampling and YCbCrA_FPX->RGBA_FPX color conversion,
//    and output data formatting of DCT output blocks, for a
//    4:1:1:4 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCrA_FPX_4114_to_RGBA_FPX_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          16,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*16*4 + CPU_CACHE_LINE-1];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    const Ipp16s* pSrcMCU[4];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrcMCU[0] = &inptr[DCTSIZE2*0]; /* Y  */
    pSrcMCU[1] = &inptr[DCTSIZE2*4]; /* Cb */
    pSrcMCU[2] = &inptr[DCTSIZE2*5]; /* Cr */
    pSrcMCU[3] = &inptr[DCTSIZE2*6]; /* A  */

    IPPCALL(ippiYCCK411ToCMYKLS_MCU_16s8u_P4C4R)(pSrcMCU,pDst,16*4);

    outptr = jprops->state.DIB_ptr;

    if(lineoffset < 0)
    {
      IppiSize c1roi;
      c1roi.width  = dibrect.right*4;
      c1roi.height = dibrect.bottom;

      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      /* to cover absent functionality of ippiMirror_8u_C4IR */
      IPPCALL(ippiMirror_8u_C1IR)(pDst,16*4,c1roi,ippAxsHorizontal);
    }

    IPPCALL(ippiCopy_8u_C4R)(pDst,16*4,outptr,lineoffset,roi);
  }

  return;
} /* YCbCrA_FPX_4114_to_RGBA_FPX_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_111_to_OTHER4_MCU
//
//  Purpose
//    Performs output data formatting of DCT output blocks for a
//    1:1:1 MCU.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//    jprops  Pointer to IJL properties persistent storage.
//    curxMCU number of current MCU in horizontal direction
//    curyMCU number of current MCU in vertical direction
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_111_to_OTHER4_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  Ipp16s*  inptr = (Ipp16s*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf1[8*8*3 + CPU_CACHE_LINE-1];
    Ipp8u buf2[8*8*3 + CPU_CACHE_LINE-1];
    Ipp8u* tmpp3 = OWN_ALIGN_PTR(buf1,CPU_CACHE_LINE);
    Ipp8u* tmpc3 = OWN_ALIGN_PTR(buf2,CPU_CACHE_LINE);
    IppiSize sz = { 8*3, 8 };
    Ipp8u* pSrc[3];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    IPPCALL(ippiAdd128_JPEG_16s8u_C1R)(inptr,8*3*sizeof(Ipp16s),tmpp3,8*3,sz);

    pSrc[0] = &tmpp3[DCTSIZE2*0];
    pSrc[1] = &tmpp3[DCTSIZE2*1];
    pSrc[2] = &tmpp3[DCTSIZE2*2];

    IPPCALL(ippiCopy_8u_P3C3R)(pSrc,8,tmpc3,8*3,roi);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;
      IPPCALL(ippiMirror_8u_C3IR)(tmpc3,8*3,roi,ippAxsHorizontal);
    }

    IPPCALL(ippiCopy_8u_C3AC4R)(tmpc3,8*3,outptr,lineoffset,roi);
    IPPCALL(ippiSet_8u_C4CR)((Ipp8u)IJL_OPAQUE,outptr+3,lineoffset,roi);
  }

  return;
} /* OTHER_111_to_OTHER4_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_1111_to_OTHER_MCU
//
//  Purpose
//    Performs output data formatting of DCT output blocks for a
//    subset of a 1:1:1:1 MCU.
//
//    This routine is a C implementation.
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Returns
//    None
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_1111_to_OTHER_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[8*8*4 + CPU_CACHE_LINE-1];
    Ipp8u* tmp = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);
    IppiSize sz = { 8*4, 8 };
    Ipp8u* pSrc[4];

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    IPPCALL(ippiAdd128_JPEG_16s8u_C1R)(inptr,8*4*sizeof(short),tmp,8*4,sz);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1IR)(&tmp[DCTSIZE2*0],8,roi,ippAxsHorizontal);
      IPPCALL(ippiMirror_8u_C1IR)(&tmp[DCTSIZE2*1],8,roi,ippAxsHorizontal);
      IPPCALL(ippiMirror_8u_C1IR)(&tmp[DCTSIZE2*2],8,roi,ippAxsHorizontal);
      IPPCALL(ippiMirror_8u_C1IR)(&tmp[DCTSIZE2*3],8,roi,ippAxsHorizontal);
    }

    pSrc[0] = &tmp[DCTSIZE2*0];
    pSrc[1] = &tmp[DCTSIZE2*1];
    pSrc[2] = &tmp[DCTSIZE2*2];
    pSrc[3] = &tmp[DCTSIZE2*3];

    IPPCALL(ippiCopy_8u_P4C4R)(pSrc,8,outptr,lineoffset,roi);
  }

  return;
} /* OTHER_1111_to_OTHER_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_111_to_OTHER_MCU
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Parameters
//    See OTHER_111_to_OTHER_Full_MCU and OTHER_111_to_OTHER for details.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_111_to_OTHER_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[8*8*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE-1);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Ch1 */
    pSrc[1] = &inptr[DCTSIZE2*1]; /* Ch2 */
    pSrc[2] = &inptr[DCTSIZE2*2]; /* Ch3 */

    IPPCALL(ippiSampleUp444LS_MCU_16s8u_P3C3R)(pSrc,pDst,8*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,8*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,8*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* OTHER_111_to_OTHER_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_411_to_OTHER_MCU
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Parameters
//    See OTHER_411_to_OTHER_Full_MCU and OTHER_411_to_OTHER for details.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_411_to_OTHER_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          16,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*16*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    outptr = jprops->state.DIB_ptr;

    pSrc[0] = &inptr[DCTSIZE2*0];
    pSrc[1] = &inptr[DCTSIZE2*4];
    pSrc[2] = &inptr[DCTSIZE2*5];

    IPPCALL(ippiSampleUp411LS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,16*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,16*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* OTHER_411_to_OTHER_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    OTHER_422_to_OTHER_MCU
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Parameters
//    See OTHER_422_to_OTHER_Full_MCU and OTHER_422_to_OTHER for details.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) OTHER_422_to_OTHER_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*8*3 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Ch1 */
    pSrc[1] = &inptr[DCTSIZE2*2]; /* Ch2 */
    pSrc[2] = &inptr[DCTSIZE2*3]; /* Ch3 */

    IPPCALL(ippiSampleUp422LS_MCU_16s8u_P3C3R)(pSrc,pDst,16*3);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C3R)(pDst,16*3,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C3R)(pDst,16*3,outptr,lineoffset,roi);
    }
  }

  return;
} /* OTHER_422_to_OTHER_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_422_to_YCbYCr_MCU
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Parameters
//    See other "fast-processing" procedures for details.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) YCbCr_422_to_YCbYCr_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          16,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  {
    IppiSize roi;
    Ipp8u buf[16*8*2 + CPU_CACHE_LINE-1];
    const Ipp16s* pSrc[3];

    Ipp8u* pDst = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

    roi.width  = dibrect.right*2;
    roi.height = dibrect.bottom;

    pSrc[0] = &inptr[DCTSIZE2*0]; /* Ch1 */
    pSrc[1] = &inptr[DCTSIZE2*2]; /* Ch2 */
    pSrc[2] = &inptr[DCTSIZE2*3]; /* Ch3 */

    IPPCALL(ippiJoin422LS_MCU_16s8u_P3C2R)(pSrc,pDst,16*2);

    if(lineoffset < 0)
    {
      lineoffset = -lineoffset;
      outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;

      IPPCALL(ippiMirror_8u_C1R)(pDst,16*2,outptr,lineoffset,roi,ippAxsHorizontal);
    }
    else
    {
      outptr = jprops->state.DIB_ptr;

      IPPCALL(ippiCopy_8u_C1R)(pDst,16*2,outptr,lineoffset,roi);
    }
  }

  return;
} /* YCbCr_422_to_YCbYCr_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Y_111_to_Y_MCU
//
//  Context
//    Used by Decoder for every decoded DCT block in an image.
//
//  Parameters
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) Y_111_to_Y_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  short*   inptr = (short*)jprops->MCUBuf;
  Ipp8u*   outptr;
  int      lineoffset = jprops->DIBLineBytes;
  IJL_RECT dibrect;
  IJLERR   jerr;

  jerr = CalcDIBRoi(
          &jprops->roi,
          8,
          8,
          curxMCU,
          curyMCU,
          jprops->DIBChannels,
          jprops->DIBLineBytes,
          jprops->DIBBytes,
          jprops->DIBWidth,
          jprops->DIBHeight,
          &(jprops->state.DIB_ptr),
          &dibrect);

  if(IJL_OK != jerr)
  {
    return;
  }

  if(lineoffset < 0)
  {
    lineoffset = -lineoffset;
    outptr = jprops->state.DIB_ptr - (dibrect.bottom - 1)*lineoffset;
  }
  else
  {
    outptr = jprops->state.DIB_ptr;
  }

  {
    IppiSize roi;
    roi.width  = dibrect.right;
    roi.height = dibrect.bottom;

    IPPCALL(ippiAdd128_JPEG_16s8u_C1R)(inptr,8*sizeof(short),outptr,lineoffset,roi);

    if(jprops->DIBHeight < 0)
    {
      IPPCALL(ippiMirror_8u_C1IR)(outptr,lineoffset,roi,ippAxsHorizontal);
    }
  }

  return;
} /* Y_111_to_Y_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Output_Interleave_General_MCU
//
//  Purpose
//    Copies data from the MCU buffer (jprops->MCUBuf) into the
//    output DIB, without color conversion or upsampling.
//
//  Context
//    Called to process one MCU of data
//
//  Returns
//    none
//
//  Parameters
//    *jprops              Pointer to global IJL properties.
//    *state               Pointer to current state of the IJL.
//    increment_outbuffer  If set, the state DIB indicator is incremented.
//                         If null, the state of the DIB is unaffected
//    curxMCU              Used by an inline variant of SETUP_PTR (above)
//                         to indicate when the right edge of an image
//                         has been reached.
//    curyMCU              Used by an inline variant of SETUP_PTR (above)
//                         to indicate when the bottom edge of an image
//                         has been reached.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(void) Output_Interleave_General_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int      i, j, k, l, m;
  Ipp16s*  inptr;
  Ipp8u*   outptr;
  IJLERR   jerr;
  IJL_RECT DIBRect;

  jerr = CalcDIBRoi(
    &jprops->roi,
    8,
    8,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(jprops->state.DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  inptr = (Ipp16s*)jprops->MCUBuf;

  if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
  {
    outptr = jprops->state.DIB_ptr + 2;
  }
  else
  {
    outptr = jprops->state.DIB_ptr;
  }

  for(i = 0; i < jprops->DIBChannels; i++)
  {
    if((i >= 1) && (jprops->JPGColor == IJL_G))
    {
      /*
      // Since the user wants to decode a grayscale image (1 channel)
      // into a >1-channel DIB, we need to fill in the "new" channels.
      // For example, if a 1-channel grayscale JPEG is decompressed
      // into a 3-byte buffer, the first byte will be correct and
      // the next two bytes will be set equal to the first channel.
      */

      for(l = DIBRect.top; l < DIBRect.bottom; l++)
      {
        for(m = DIBRect.left; m < DIBRect.right; m++)
        {
          if(i < 3)
          {
            outptr[(l*jprops->DIBLineBytes) + (m*jprops->DIBChannels)] =
            outptr[(l*jprops->DIBLineBytes) + (m*jprops->DIBChannels) - 1];
          }
          else  // i == 3
          {
            outptr[(l*jprops->DIBLineBytes) + (m*jprops->DIBChannels)] =
              (Ipp8u)CLIP(IJL_OPAQUE + 128);
          }
        }
      }
    }
    else if(i < jprops->jframe.ncomps)
    {
      for(j = 0; j < jprops->jframe.comps[i].vsampling; j++)
      {
        for(k = 0; k < jprops->jframe.comps[i].hsampling; k++)
        {
          for(l = DIBRect.top; l < DIBRect.bottom; l++)
          {
            for(m = DIBRect.left; m < DIBRect.right; m++)
            {
              outptr[(l*jprops->DIBLineBytes) + (m*jprops->DIBChannels)] =
                (Ipp8u)CLIP(inptr[(l*8) + m] + 128);
            }
          }

          inptr += DCTSIZE2;
        }
      }
    }
    else if((i == 3) && (jprops->DIBColor == IJL_RGBA_FPX))
    {
      for(l = DIBRect.top; l < DIBRect.bottom; l++)
      {
        for(m = DIBRect.left; m < DIBRect.right; m++)
        {
          outptr[(l*jprops->DIBLineBytes) + (m*jprops->DIBChannels)] =
            (Ipp8u)CLIP(IJL_OPAQUE + 128);
        }
      }

      inptr += DCTSIZE2;
    }

    if((jprops->DIBColor == IJL_BGR) && (jprops->JPGColor == IJL_RGB))
    {
      outptr--;
    }
    else
    {
      outptr++;
    }
  }

  return;
} /* Output_Interleave_General_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    US_General_MCU
//
//  Purpose
//    Performs general upsampling on an MCU of data.
//    Upsampling is "box" upsampling (i.e., data is
//    replicated n times, where n is the upsampling
//    factor).  Works for any JPEG-compliant upsampling
//    factors (up to 4:1 across four channels).
//    Data is assumed pixel interleaved.
//
//  Context
//    Called to process one MCU of data
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

OWNFUN(void) US_General_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int      i;
  int      j;
  int      k;
  int      l;
  int      m;
  int      q;
  int      h;
  int      v;
  int      s;
  int      rvalue;
  int      tvalue;
  int      lvalue;
  int      bvalue;
  int      invalue;
  int      ltvalue;
  int      rtvalue;
  int      lbvalue;
  int      rbvalue;
  int      sampType;
  int      h_expand;
  int      v_expand;
  int      pixwidth;
  int      pixheight;
  int      comp_i_vsampling;
  int      comp_i_hsampling;
  int      scan_comps;
  int      scan_maxhsampling;
  int      scan_maxvsampling;
  Ipp16s*  inptr   = NULL;
  Ipp8u*   outptr  = NULL;
  Ipp8u*   tempptr = NULL;
  Ipp16s*  leftBorderMCU   = NULL;
  Ipp16s*  rightBorderMCU  = NULL;
  Ipp16s*  topBorderMCU    = NULL;
  Ipp16s*  bottomBorderMCU = NULL;
  Ipp16s*  ltBorderMCU = NULL;
  Ipp16s*  rtBorderMCU = NULL;
  Ipp16s*  lbBorderMCU = NULL;
  Ipp16s*  rbBorderMCU = NULL;
  IJLERR   jerr;
  IJL_RECT DIBRect;
  Ipp8u*   temMCUBuf;
  Ipp8u    buf[DCTSIZE2 * MAX_BLKS_PER_MCU * sizeof(Ipp16s) + CPU_CACHE_LINE-1];

  temMCUBuf = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

  /* Non-interleaved data. */
  if(jprops->jscan->ncomps == 1)
  {
    if(jprops->state.cur_scan_comp >= jprops->DIBChannels)
      return;

    scan_maxhsampling = jprops->jframe.max_hsampling / jprops->jscan->comps[0].hsampling;
    scan_maxvsampling = jprops->jframe.max_vsampling / jprops->jscan->comps[0].vsampling;

    pixwidth  = 8 * scan_maxhsampling;
    pixheight = 8 * scan_maxvsampling;
  }
  else
  {
    scan_maxhsampling = jprops->jframe.max_hsampling;
    scan_maxvsampling = jprops->jframe.max_vsampling;

    pixwidth  = 8 * scan_maxhsampling;
    pixheight = 8 * scan_maxvsampling;
  }

  /* Assume all upsampling is separable and assume MCUs are */
  /* all in the same scan (no progressive or hierarchical). */

  inptr = (Ipp16s*)jprops->MCUBuf;

  leftBorderMCU   = NULL;
  rightBorderMCU  = NULL;
  topBorderMCU    = NULL;
  bottomBorderMCU = NULL;
  ltBorderMCU     = NULL;
  rtBorderMCU     = NULL;
  lbBorderMCU     = NULL;
  rbBorderMCU     = NULL;

  sampType = 0;

  if(jprops->upsampling_type == IJL_TRIANGLE_FILTER)
  {
    if(jprops->jframe.max_hsampling == 2 &&
       jprops->jframe.max_vsampling == 1)
    {
      sampType = 1;
      if(jprops->jinterleaveType)
      {
        l = 1;
      }
      else
      {
        for(k = jprops->jframe.ncomps, l = 0; k--;)
        {
          l += jprops->jframe.comps[k].hsampling *
               jprops->jframe.comps[k].vsampling;
        }
      }

      l <<= 6;

      leftBorderMCU  = jprops->sampling_state_ptr->cur_row + (curxMCU+0) * l;
      rightBorderMCU = jprops->sampling_state_ptr->cur_row + (curxMCU+2) * l;
    }

    if(jprops->jframe.max_hsampling == 2 &&
       jprops->jframe.max_vsampling == 2)
    {
      sampType = 2;
      if(jprops->jinterleaveType)
      {
        l = 1;
      }
      else
      {
        for(k = jprops->jframe.ncomps, l = 0; k--;)
        {
          l += jprops->jframe.comps[k].hsampling *
               jprops->jframe.comps[k].vsampling;
        }
      }

      l <<= 6;

      j = jprops->numxMCUs + 2;
      m = curxMCU + 1;
      s = curxMCU + 2;

      topBorderMCU    = jprops->sampling_state_ptr->top_row + m*l;
      ltBorderMCU     = jprops->sampling_state_ptr->top_row + curxMCU*l;
      rtBorderMCU     = jprops->sampling_state_ptr->top_row + s*l;

      leftBorderMCU   = jprops->sampling_state_ptr->cur_row + curxMCU*l;
      rightBorderMCU  = jprops->sampling_state_ptr->cur_row + s*l;

      bottomBorderMCU = jprops->sampling_state_ptr->bottom_row + m*l;
      lbBorderMCU     = jprops->sampling_state_ptr->bottom_row + curxMCU*l;
      rbBorderMCU     = jprops->sampling_state_ptr->bottom_row + s*l;
    }
  }

  jerr = CalcDIBRoi(
    &jprops->roi,
    pixwidth,
    pixheight,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(jprops->state.DIB_ptr),
    &DIBRect);

  if(IJL_OK != jerr)
  {
    return;
  }

  /*
  // Offset from the current (x,y) pixel location in the
  // output buffer is determined by the index of the
  // current scan component.
  */

  if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
  {
    outptr = jprops->state.DIB_ptr;

    if(jprops->state.cur_scan_comp == 0)
    {
      outptr += 2;
    }
    else if(jprops->state.cur_scan_comp == 1)
    {
      outptr += 1;
    }
  }
  else
  {
    outptr = jprops->state.DIB_ptr + jprops->state.cur_scan_comp;
  }

  scan_comps = jprops->jscan->ncomps;

  /* Loop over the components in the scan. */
  for(i = 0; i < min(scan_comps,jprops->DIBChannels); i++)
  {
    if(scan_comps == 1 && !jprops->progressive_found)
    {
      comp_i_vsampling = 1;
      comp_i_hsampling = 1;
    }
    else
    {
      comp_i_vsampling = jprops->jscan->comps[i].vsampling;
      comp_i_hsampling = jprops->jscan->comps[i].hsampling;
    }

    h_expand = jprops->jframe.max_hsampling / jprops->jscan->comps[i].hsampling;
    v_expand = jprops->jframe.max_vsampling / jprops->jscan->comps[i].vsampling;

    if(sampType == 1) /* as 422 */
    {
      /* Again, loop over the components in the scan, and not the frame. */
      tempptr = &temMCUBuf[0];

      for(k = 0; k < comp_i_hsampling; k++)
      {
        for(l = 0; l < 256; l += 32)
        {

          if(h_expand == 2)
          {
            invalue = inptr[0]*3;
            tempptr[l + 0] = (Ipp8u)CLIP((invalue + leftBorderMCU[7] + 1 + 4*128) >> 2);
            tempptr[l + 1] = (Ipp8u)CLIP((invalue + inptr[1] + 2 + 4*128) >> 2);

            for(q = 1; q < 7; q++)
            {
              m = q << 1;
              invalue = inptr[q]*3;
              tempptr[l + m + 0] = (Ipp8u)CLIP((invalue + inptr[q-1] + 1 + 4*128) >> 2);
              tempptr[l + m + 1] = (Ipp8u)CLIP((invalue + inptr[q+1] + 2 + 4*128) >> 2);
            }

            invalue = inptr[7] * 3;
            tempptr[l + 14] = (Ipp8u)CLIP((invalue + inptr[6] + 1 + 4*128) >> 2);
            tempptr[l + 15] = (Ipp8u)CLIP((invalue + rightBorderMCU[0] + 2 + 4*128) >> 2);
          }
          else
          {
            for(m = 0; m < 8; m ++)
            {
              tempptr[l + m] = (Ipp8u)CLIP(inptr[m] + 128);
            }
          }
          inptr          += 8;
          rightBorderMCU += 8;
          leftBorderMCU  += 8;
        }
        tempptr += (8 * h_expand);
      }
    } /* 422 */
    else if(sampType == 2) /* as 411 */
    {
      /* Again, loop over the components in the scan, and not the frame. */
      for(j = 0; j < comp_i_vsampling; j++)
      {
        tempptr = &temMCUBuf[0] + (j * 8 * 32 * v_expand);

        for(k = 0; k < comp_i_hsampling; k++)
        {
          if(h_expand == 2 && v_expand == 2)
          {
            for(l = 0,  s = 8; s--; l += 64)
            {
              invalue = inptr[0] * 9;
              lvalue  = leftBorderMCU[7] * 3;
              rvalue  = inptr[1] * 3;

              if(l != 0)
              {
                tvalue  = inptr[-8] *3;
                ltvalue = leftBorderMCU[-1];
                rtvalue = inptr[-7];
              }
              else
              {
                tvalue  = topBorderMCU[56]*3;
                ltvalue = ltBorderMCU[63];
                rtvalue = topBorderMCU[57];
              }
              if(s != 0)
              {
                bvalue  = inptr[8] * 3;
                lbvalue = leftBorderMCU[15];
                rbvalue = inptr[9];
              }
              else
              {
                bvalue  = bottomBorderMCU[0]*3;
                lbvalue = lbBorderMCU[7];
                rbvalue = bottomBorderMCU[1];
              }

              tempptr[l +  0] = (Ipp8u)CLIP((invalue + lvalue + tvalue + ltvalue + 8 + 16*128) >> 4);
              tempptr[l +  1] = (Ipp8u)CLIP((invalue + rvalue + tvalue + rtvalue + 7 + 16*128) >> 4);
              tempptr[l + 32] = (Ipp8u)CLIP((invalue + lvalue + bvalue + lbvalue + 8 + 16*128) >> 4);
              tempptr[l + 33] = (Ipp8u)CLIP((invalue + rvalue + bvalue + rbvalue + 7 + 16*128) >> 4);

              inptr++;
              topBorderMCU++;
              bottomBorderMCU++;

              for(m = 2; m < 14; m += 2)
              {
                /* 3/4 * nearer pixel + 1/4 * further pixel in each */
                /* dimension, thus 9/16, 3/16, 3/16, 1/16 overall   */

                invalue = inptr[0] * 9;
                lvalue  = inptr[-1] * 3;
                rvalue  = inptr[1] * 3;

                if(l != 0)
                {
                  tvalue  = inptr[-8] * 3;
                  ltvalue = inptr[-9];
                  rtvalue = inptr[-7];
                }
                else
                {
                  tvalue  = topBorderMCU[56] * 3;
                  ltvalue = topBorderMCU[55];
                  rtvalue = topBorderMCU[57];
                }
                if(s != 0)
                {
                  bvalue  = inptr[8] * 3;
                  lbvalue = inptr[7];
                  rbvalue = inptr[9];
                }
                else
                {
                  bvalue  = bottomBorderMCU[0] * 3;
                  lbvalue = bottomBorderMCU[-1];
                  rbvalue = bottomBorderMCU[1];
                }

                tempptr[l + m +  0] = (Ipp8u)CLIP((invalue + lvalue + tvalue + ltvalue + 8 + 16*128) >> 4);
                tempptr[l + m +  1] = (Ipp8u)CLIP((invalue + rvalue + tvalue + rtvalue + 7 + 16*128) >> 4);
                tempptr[l + m + 32] = (Ipp8u)CLIP((invalue + lvalue + bvalue + lbvalue + 8 + 16*128) >> 4);
                tempptr[l + m + 33] = (Ipp8u)CLIP((invalue + rvalue + bvalue + rbvalue + 7 + 16*128) >> 4);

                inptr++;
                topBorderMCU++;
                bottomBorderMCU++;
              }

              invalue = inptr[0] * 9;
              lvalue  = inptr[-1] *3;
              rvalue  = rightBorderMCU[0]*3;
              if(l != 0)
              {
                tvalue  = inptr[-8] * 3;
                ltvalue = inptr[-9];
                rtvalue = rightBorderMCU[-8];
              }
              else
              {
                tvalue  = topBorderMCU[56] * 3;
                ltvalue = topBorderMCU[55];
                rtvalue = rtBorderMCU[56];
              }
              if(s != 0)
              {
                bvalue  = inptr[8] * 3;
                lbvalue = inptr[7];
                rbvalue = rightBorderMCU[8];
              }
              else
              {
                bvalue  = bottomBorderMCU[0] * 3;
                lbvalue = bottomBorderMCU[-1];
                rbvalue = rbBorderMCU[0];
              }

              tempptr[l + 14] = (Ipp8u)CLIP((invalue + lvalue + tvalue + ltvalue + 8 + 16*128) >> 4);
              tempptr[l + 15] = (Ipp8u)CLIP((invalue + rvalue + tvalue + rtvalue + 7 + 16*128) >> 4);
              tempptr[l + 46] = (Ipp8u)CLIP((invalue + lvalue + bvalue + lbvalue + 8 + 16*128) >> 4);
              tempptr[l + 47] = (Ipp8u)CLIP((invalue + rvalue + bvalue + rbvalue + 7 + 16*128) >> 4);

              inptr++;
              topBorderMCU++;
              bottomBorderMCU++;

              rightBorderMCU  += 8;
              leftBorderMCU   += 8;
              topBorderMCU    -= 8;
              bottomBorderMCU -= 8;
            }

            tempptr         += 16;
            topBorderMCU    += DCTSIZE2;
            bottomBorderMCU += DCTSIZE2;
            ltBorderMCU     += DCTSIZE2;
            rtBorderMCU     += DCTSIZE2;
            lbBorderMCU     += DCTSIZE2;
            rbBorderMCU     += DCTSIZE2;
          }
          else
          {
            /* Again, loop over the components in the scan, and not the frame. */
            for(j = 0; j < comp_i_vsampling; j++)
            {
              tempptr = &temMCUBuf[0] + (j * 8 * 32 * v_expand);

              for(k = 0; k < comp_i_hsampling; k++)
              {
                for(l = 0; l < (8 * 32 * v_expand); l += (32 * v_expand))
                {
                  for(m = 0; m < (8 * h_expand); m += h_expand)
                  {
                    /* The v and h loops are correct. No need to modify. */
                    invalue = *inptr++;

                    for(v = 0; v < (v_expand * 32); v += 32)
                    {
                      for(h = 0; h < h_expand; h++)
                      {
                        tempptr[l + m + v + h] = (Ipp8u)CLIP(invalue + 128);
                      }
                    }
                  }
                }
                tempptr         += (8 * h_expand);
                topBorderMCU    += DCTSIZE2;
                leftBorderMCU   += DCTSIZE2;
                rightBorderMCU  += DCTSIZE2;
                bottomBorderMCU += DCTSIZE2;
                ltBorderMCU     += DCTSIZE2;
                rtBorderMCU     += DCTSIZE2;
                lbBorderMCU     += DCTSIZE2;
                rbBorderMCU     += DCTSIZE2;
              }
            }
          }
        }
      }
    } /* 411 */
    else
    {
      /* Again, loop over the components in the scan, and not the frame. */
      for(j = 0; j < comp_i_vsampling; j++)
      {
        tempptr = &temMCUBuf[0] + (j * 8 * 32 * v_expand);

        for(k = 0; k < comp_i_hsampling; k++)
        {
          for(l = 0; l < (8 * 32 * v_expand); l += (32 * v_expand))
          {
            for(m = 0; m < (8 * h_expand); m += h_expand)
            {
              /* The v and h loops are correct. No need to modify. */
              invalue = *inptr++;

              for(v = 0; v < (v_expand * 32); v += 32)
              {
                for(h = 0; h < h_expand; h++)
                {
                  tempptr[l + m + v + h] = (Ipp8u)CLIP(invalue + 128);
                }
              }
            }
          }
          tempptr += (8 * h_expand);
        }
      }
    }

    tempptr = temMCUBuf;

    /* Convert temporary buffer to output buffer. */
    Interleave_To_Output(jprops,&DIBRect,i,outptr,tempptr);

    /* Increment to the next channel. */
    if((jprops->DIBColor == IJL_BGR) && (jprops->JPGColor == IJL_RGB))
    {
      outptr--;
    }
    else
    {
      outptr++;
    }
  }

  if(jprops->jinterleaveType != 0)
  {
    i = (jprops->state.cur_scan_comp == (jprops->JPGChannels-1)) ? jprops->JPGChannels : jprops->DIBChannels;
  }

  if(jprops->DIBColor != IJL_OTHER || jprops->JPGColor != IJL_OTHER)
  {
    for(; i < jprops->DIBChannels; i++)
    {
      /* Convert temporary buffer to output buffer. */
      Interleave_To_Output(jprops,&DIBRect,i,outptr,tempptr);

      /* Increment to the next channel. */
      if(jprops->DIBColor == IJL_BGR && jprops->JPGColor == IJL_RGB)
      {
        outptr--;
      }
      else
      {
        outptr++;
      }
    }
  }

  return;
} /* US_General_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Put_MCU_To_YCbYCr
//
//  Purpose
//    Puts an MCU of YCbCr 4:2:2 data to DIB buffer in YCbYCr format.
//
//  Context
//    only for non_interleaved images
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

OWNFUN(void) Put_MCU_To_YCbYCr(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int      i, j, k;
  IJLERR   jerr;
  IJL_RECT DIBRect;
  Ipp16s*  inptr;
  Ipp8u*   temMCUBuf;
  Ipp8u    buf[DCTSIZE2 + CPU_CACHE_LINE-1];

  temMCUBuf = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

  inptr = (Ipp16s*)jprops->MCUBuf;

  for(j = 0; j < 8; j++)
  {
    for(i = 0; i < 8; i++)
    {
      temMCUBuf[j*8 + i] = (Ipp8u)(*inptr++ + 128);
    }
  }

  if(jprops->state.cur_scan_comp == 0)
  {
    jerr = CalcDIBRoi(
    &jprops->roi,
    8, 8,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(jprops->state.DIB_ptr),
    &DIBRect);

    if(IJL_OK != jerr)
    {
      return;
    }

    for(j = DIBRect.top; j < DIBRect.bottom; j++)
    {
      for(k = DIBRect.left; k < DIBRect.right; k++)
      {
        jprops->state.DIB_ptr[(j * jprops->DIBLineBytes) + (k * 2)] =
          temMCUBuf[(j * 8) + k];
      }
    }
  }
  else
  {
    jerr = CalcDIBRoi(
      &jprops->roi,
      16,
      8,
      curxMCU,
      curyMCU,
      2,
      jprops->DIBLineBytes,
      jprops->DIBBytes,
      jprops->DIBWidth,
      jprops->DIBHeight,
      &(jprops->state.DIB_ptr),
      &DIBRect);

    if(IJL_OK != jerr)
    {
      return;
    }

    /* Put data to output buffer. */
    if(jprops->state.cur_scan_comp == 2)
    {
      for(j = DIBRect.top; j < DIBRect.bottom; j++)
      {
        for(k = DIBRect.left; k < DIBRect.right; k += 2)
        {
          jprops->state.DIB_ptr[(j * jprops->DIBLineBytes) + (k * 2) + 3] =
            temMCUBuf[(j * 8) + (k>>1)];
        }
      }
    }
    else
    {
      for(j = DIBRect.top; j < DIBRect.bottom; j++)
      {
        for(k = DIBRect.left; k < DIBRect.right; k += 2)
        {
          jprops->state.DIB_ptr[(j * jprops->DIBLineBytes) + (k * 2) + 1] =
            temMCUBuf[(j * 8) + (k>>1)];
        }
      }
    }
  }

  return;
} /* Put_MCU_To_YCbYCr() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    US_CC_General_YCbCr_MCU
//
//  Purpose
//    Process one MCU of data, including upsampling to the
//    output DIB followed by YCbCr->RGB color conversion.
//    This routine works for all flavors of single-scan
//    baseline JPEG, so it is painfully slow.
//
//  Context
//    Called to process one MCU of data
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

OWNFUN(void) US_CC_General_YCbCr_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  /* Call the general upsampling algorithm. */
  if(FALSE == jprops->progressive_found)
    US_General_MCU(jprops, curxMCU, curyMCU);
  else
    US_General_P_MCU(jprops, curxMCU, curyMCU);

  /* Call the color conversion algorithm. */
  CC_YCbCr_InPlace(jprops, &jprops->state, curxMCU, curyMCU);

  return;
} /* US_CC_General_YCbCr_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    CC_General_YCbCr_MCU
//
//  Purpose
//    Process 1 MCU of data including interleaving to the
//    output DIB and subsequent YCbCr->RGB color conversion.
//    Slightly faster than the general case,
//    US_CC_General_YCbCr, which also performs upsampling.
//
//  Context
//    Called to process one MCU of data
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

OWNFUN(void) CC_General_YCbCr_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  /* Call the general non-upsampled interleave-to-output routine. */
  Output_Interleave_General_MCU(jprops, curxMCU, curyMCU);

  /* Call the color conversion algorithm. */
  CC_YCbCr_InPlace(jprops, &jprops->state, curxMCU, curyMCU);

  return;
} /* CC_General_YCbCr_MCU() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    YCbCr_422_to_YCbYCr_NI_MCU
//
//  Purpose
//    Puts an MCU of YCbCr 4:2:2 data to DIB buffer in YCbYCr format.
//
//  Context
//    only for non_interleaved images
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

OWNFUN(void) YCbCr_422_to_YCbYCr_NI_MCU(
  JPEG_PROPERTIES* jprops,
  int              curxMCU,
  int              curyMCU)
{
  int      i, j, k;
  IJLERR   jerr;
  IJL_RECT DIBRect;
  Ipp16s*  inptr;
  Ipp8u*   temMCUBuf;
  Ipp8u    buf[DCTSIZE2 + CPU_CACHE_LINE-1];

  temMCUBuf = OWN_ALIGN_PTR(buf,CPU_CACHE_LINE);

  inptr = (Ipp16s*)jprops->MCUBuf;

  for(j = 0; j < 8; j++)
  {
    for(i = 0; i < 8; i++)
    {
      temMCUBuf[j*8 + i] = (Ipp8u)(*inptr++ + 128);
    }
  }

  if(jprops->state.cur_scan_comp == 0)
  {
    jerr = CalcDIBRoi(
    &jprops->roi,
    8, 8,
    curxMCU,
    curyMCU,
    jprops->DIBChannels,
    jprops->DIBLineBytes,
    jprops->DIBBytes,
    jprops->DIBWidth,
    jprops->DIBHeight,
    &(jprops->state.DIB_ptr),
    &DIBRect);

    if(IJL_OK != jerr)
    {
      return;
    }

    for(j = DIBRect.top; j < DIBRect.bottom; j++)
    {
      for(k = DIBRect.left; k < DIBRect.right; k++)
      {
        jprops->state.DIB_ptr[(j * jprops->DIBLineBytes) + (k * 2)] =
          temMCUBuf[(j * 8) + k];
      }
    }
  }
  else
  {
    jerr = CalcDIBRoi(
      &jprops->roi,
      16,
      8,
      curxMCU,
      curyMCU,
      2,
      jprops->DIBLineBytes,
      jprops->DIBBytes,
      jprops->DIBWidth,
      jprops->DIBHeight,
      &(jprops->state.DIB_ptr),
      &DIBRect);

    if(IJL_OK != jerr)
    {
      return;
    }

    /* Put data to output buffer. */
    if(jprops->state.cur_scan_comp == 2)
    {
      for(j = DIBRect.top; j < DIBRect.bottom; j++)
      {
        for(k = DIBRect.left; k < DIBRect.right; k += 2)
        {
          jprops->state.DIB_ptr[(j * jprops->DIBLineBytes) + (k * 2) + 3] =
            temMCUBuf[(j * 8) + (k>>1)];
        }
      }
    }
    else
    {
      for(j = DIBRect.top; j < DIBRect.bottom; j++)
      {
        for(k = DIBRect.left; k < DIBRect.right; k += 2)
        {
          jprops->state.DIB_ptr[(j * jprops->DIBLineBytes) + (k * 2) + 1] =
            temMCUBuf[(j * 8) + (k>>1)];
        }
      }
    }
  }

  return;
} /* YCbCr_422_to_YCbYCr_NI_MCU() */

#undef CC_YCBCR_R
#undef CC_YCBCR_G
#undef CC_YCBCR_B
