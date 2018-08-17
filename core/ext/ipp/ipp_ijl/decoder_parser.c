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
#ifndef __DECODER_PARSER_H__
#include "parser/decoder_parser.h"
#endif




#ifdef __INTEL_COMPILER
#pragma warning(disable:171)
#pragma warning(disable:174)
#pragma warning(disable:188)
#pragma warning(disable:424)
#endif


//EXC_INIT();


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Get_Next_Marker
//
//  Purpose
//    Search through the input JPEG datastream until a marker
//    is found. A marker is an 0xFFXX, where XX is _not_ 00,
//    or FF. If the marker is an EOF marker, a special return is invoked
//
//
//  Context
//    Called by the main JPEG decoder routine to get the start of the next
//    unread chunk of data.
//
//  Returns
//    The marker found, or MARKER_END_FILE if an EOF marker is detected.
//
//  Parameters
//    pState  The pointer to current IJL working state
//    pMarker The pointer to variable to store marker
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Get_Next_Marker(
  STATE*      pState,
  IJL_MARKER* pMarker)
{
  int     byte0;
  int     byte1;
  IJLERR  jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"-> DP_Get_Next_Marker()\n");

  if(pState->unread_marker)
  {
    /* already have marker */
    TRACE1(trINFO,"Have unread marker - 0x%08X\n",pState->unread_marker);
    pMarker[0]            = (IJL_MARKER)pState->unread_marker;
    pState->unread_marker = 0;
    goto Exit;
  }
  else
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
          pMarker[0] = (IJL_MARKER)byte1;
          break;
        }
        else
        {
          pState->cur_entropy_ptr--;
          pState->entropy_bytes_processed--;
          pState->entropy_bytes_left++;
        }
      }
    }
  }

Exit:

  TRACE0(trCALL|trAFTER,"<- DP_Get_Next_Marker()\n");

  return jerr;
} /* DP_Get_Next_Marker() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Skip
//
//  Purpose
//    Skip past a segment of data
//
//  Context
//    Called to ignore unsupported segment types.
//
//  Returns
//    a valid error code, or 0 if ok.
//
//  Parameters
//    pState  The pointer to current IJL working state
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Skip(
  STATE* pState)
{
  int     i;
  int     len;
  int     byte;
  IJLERR  jerr = IJL_OK;

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
    goto Exit;

  if(len < 2)
  {
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  len -= 2;

  for(i = 0; i < len; i++)
  {
    jerr = _READ_BYTE(pState,&byte);
    if(IJL_OK != jerr)
      goto Exit;
  }

  pState->unread_marker = MARKER_NONE;

Exit:

  return jerr;
} /* DP_Skip() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Parse_APP
//
//  Purpose
//    When an application-specific marker is discovered, this routine
//    returns a pointer to a buffer containing that segment.
//
//  Context
//    Called from the main Decoder routine
//
//  Returns
//    A valid error, or IJL_OK if no error.
//
//  Parameters
//    pState   The pointer to current IJL working state
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_APP(
  STATE* pState)
{
  return DP_Skip(pState);
} /* DP_Parse_APP() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    DP_Parse_DRI
//
//  Purpose:
//    Gets the restart interval
//
//  Context:
//    Called from the main Decoder Routine
//
//  Returns:
//    Set to a valid error code, or 0 if no error.
//
//  Parameters:
//    pState        The current IJL working state
//    pRstInterval  returns the restart interval, or 0 if an error.
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_DRI(
  STATE* pState,
  int*   pRstInterval)
{
  int len;
  IJLERR jerr;

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
    return jerr;

  jerr = _READ_WORD(pState,pRstInterval);
  if(IJL_OK != jerr)
    return jerr;

  return IJL_OK;
} /* DP_Parse_DRI() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    DP_Parse_SOF
//
//  Purpose:
//    Get the frame specific properties from a frame segment.
//    Set up related frame properties from these.
//
//  Context:
//    Called by IJL_Decode
//
//  Returns:
//    IJL_OK if success, error code if not
//
//  Parameters:
//    STATE*           state      The current IJL working state
//    JPEG_PROPERTIES* jprops The IJL properties structure.
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_SOF(
  STATE*           pState,
  JPEG_PROPERTIES* jprops)
{
  int              i;
  int              len;
  int              maxv;
  int              maxh;
  int              temp;
  int              length;
  IJLERR           jerr;

  TRACE0(trCALL|trBEFORE,"enter in DP_Parse_SOF\n");

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
  {
    goto Exit;
  }

  if(len < 2)
  {
    TRACE1(trERROR,"ERROR: len is %d\n",len);
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  len -= 2;

  IPPCALL(ippsZero_8u)((Ipp8u*)jprops->jframe.comps,sizeof(FRAME_COMPONENT)*MAX_COMP_PER_SCAN);

  jerr = _READ_BYTE(pState,&temp);
  if(IJL_OK != jerr)
    goto Exit;

  jprops->jframe.precision = temp;

  if(jprops->jframe.precision != 8)
  {
    TRACE1(trERROR,"ERROR: jprops->jframe.precision is %d\n",jprops->jframe.precision);
    jerr = IJL_UNSUPPORTED_FRAME;
    goto Exit;
  }

  jerr = _READ_WORD(pState,&temp);
  if(IJL_OK != jerr)
    goto Exit;

  jprops->jframe.height = temp;

  jerr = _READ_WORD(pState,&temp);
  if(IJL_OK != jerr)
    goto Exit;

  jprops->jframe.width = temp;

  jerr = _READ_BYTE(pState,&temp);
  if(IJL_OK != jerr)
    goto Exit;

  jprops->jframe.ncomps = temp;

  if(jprops->jframe.height == 0 || jprops->jframe.width == 0)
  {
    TRACE1(trERROR,"ERROR: jprops->jframe.height is %d\n",
      jprops->jframe.height);
    TRACE1(trERROR,"ERROR: jprops->jframe.width is %d\n",
      jprops->jframe.width);
    jerr = IJL_ERR_DNL;
    goto Exit;
  }

  maxh = 0;
  maxv = 0;

  if(jprops->jframe.ncomps > MAX_COMP_PER_SCAN)
  {
    if(NULL != jprops->jframe.comps)
    {
      ippFree(jprops->jframe.comps);
      jprops->jframe.comps = NULL;
    }

    jprops->jframe.comps = ippMalloc(sizeof(FRAME_COMPONENT)*jprops->jframe.ncomps);

    if(NULL == jprops->jframe.comps)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for FRAME_COMPONENTs\n",
        sizeof(FRAME_COMPONENT)*jprops->jframe.ncomps);
      jerr = IJL_MEMORY_ERROR;
      goto Exit;
    }

    IPPCALL(ippsZero_8u)(
      (Ipp8u*)jprops->jframe.comps,
      sizeof(FRAME_COMPONENT)*jprops->jframe.ncomps);
  }

  for(i = 0; i < jprops->jframe.ncomps; i++)
  {
    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->jframe.comps[i].ident = temp;

    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->jframe.comps[i].hsampling = temp >> 4;
    jprops->jframe.comps[i].vsampling = temp & 0x0f;

    if(jprops->jframe.comps[i].hsampling == 0 || jprops->jframe.comps[i].vsampling == 0)
    {
      TRACE2(trERROR,"ERROR: jprops->jframe.comps[%d].hsampling is %d\n",
        i,jprops->jframe.comps[i].hsampling);
      TRACE2(trERROR,"ERROR: jprops->jframe.comps[%d].vsampling is %d\n",
        i,jprops->jframe.width);
      jerr = IJL_ERR_SOF;
      goto Exit;
    }

    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->jframe.comps[i].quant_sel = temp;

    if(maxh < jprops->jframe.comps[i].hsampling)
    {
      maxh = jprops->jframe.comps[i].hsampling;
    }

    if(maxv < jprops->jframe.comps[i].vsampling)
    {
      maxv = jprops->jframe.comps[i].vsampling;
    }
  }

  length                  = maxh * 8;
  jprops->jframe.horMCU   = (jprops->jframe.width + (length-1)) / length;
  length                  = maxv * 8;
  jprops->jframe.totalMCU = (long)jprops->jframe.horMCU*((jprops->jframe.height+(length-1))/length);

Exit:

  TRACE0(trCALL|trAFTER,"leave DP_Parse_SOF\n");

  return jerr;
} /* DP_Parse_SOF() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name:
//    DP_Parse_APP14()
//
//  Purpose:
//    Parse Adobe APP14 application extension segment.
//    this segment gives insight to the meaning of the color space
//    represented in the JPEG file. APP14 is seldom included in JFIF files.
//
//  Context:
//    Called from the main Decoder routine.
//
//  Returns:
//    A valid error code, or 0 if no error
//
//  Parameters:
//    pAdobeFound
//    pAdobeVersion
//    pAdobeFlags0
//    pAdobeFlags1
//    pAdobeXform
//    pState)
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_APP14(
  int*    pAdobeFound,
  Ipp16u* pAdobeVersion,
  Ipp16u* pAdobeFlags0,
  Ipp16u* pAdobeFlags1,
  int*    pAdobeXform,
  STATE*  pState)
{
  int     len;
  int     id[5];
  int     temp;
  IJLERR  jerr = IJL_OK;

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
    goto Exit;

  if(len < 2)
  {
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  len -= 2;

  if(len >= 12)
  {
    jerr = _READ_BYTE(pState,&id[0]);
    if(IJL_OK != jerr)
      goto Exit;

    jerr = _READ_BYTE(pState,&id[1]);
    if(IJL_OK != jerr)
      goto Exit;

    jerr = _READ_BYTE(pState,&id[2]);
    if(IJL_OK != jerr)
      goto Exit;

    jerr = _READ_BYTE(pState,&id[3]);
    if(IJL_OK != jerr)
      goto Exit;

    jerr = _READ_BYTE(pState,&id[4]);
    if(IJL_OK != jerr)
      goto Exit;

    len -= 5;

    if(id[0] == 0x41 &&
       id[1] == 0x64 &&
       id[2] == 0x6f &&
       id[3] == 0x62 &&
       id[4] == 0x65)
    {
      /* Found Adobe special code */
      jerr = _READ_WORD(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      pAdobeVersion[0] = (Ipp16u)temp;

      jerr = _READ_WORD(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      pAdobeFlags0[0] = (Ipp16u)temp;

      jerr = _READ_WORD(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      pAdobeFlags1[0] = (Ipp16u)temp;

      jerr = _READ_BYTE(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      pAdobeXform[0] = (Ipp8u)temp;

      pAdobeFound[0] = 1;

      len -= 7;
    }
  }

  while(len)
  {
    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    len--;
  }

Exit:

  return jerr;
} /* DP_Parse_APP14() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Parse_APP0()
//
//  Purpose
//    Parse JFIF APP0 application segment.
//
//  Context
//    Called from the main Decoder routine.
//
//  Returns
//    A valid error code, or 0 if no error
//
//  Parameters
//    JPEG_PROPERTIES* jprops      The IJL properties structure.
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_APP0(
  STATE* pState,
  JPEG_PROPERTIES* jprops)
{
  int     i;
  int     j;
  int     t[3];
  int     id[5];
  int     len;
  int     temp;
  IJLERR  jerr;

  TRACE0(trCALL|trBEFORE,"enter in DP_Parse_APP0\n");

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: Get_Segment_Length\n");
    goto Exit;
  }

  if(len < 2)
  {
    TRACE1(trERROR,"ERROR: len is %d\n",len);
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  len -= 2;

  if(len < 5)
  {
    goto Skip;
  }

  jerr = _READ_BYTE(pState,&id[0]);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _READ_BYTE(pState,&id[1]);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _READ_BYTE(pState,&id[2]);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _READ_BYTE(pState,&id[3]);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _READ_BYTE(pState,&id[4]);
  if(IJL_OK != jerr)
    goto Exit;

  len -= 5;

  /* test the marker for JFIF compliance. */
  if(id[0] == 0x4A &&
     id[1] == 0x46 &&
     id[2] == 0x49 &&
     id[3] == 0x46 &&
     id[4] == 0x00)
  {
    if(FALSE == jprops->needframe)
    {
      /* if a frame has been seen */
      goto Exit;
    }

    jprops->jfif_app0_detected = 1;

    /* store version */
    jerr = _READ_WORD(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->jfif_app0_version = (Ipp16u)temp;

    /* store units */
    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->jfif_app0_units = (Ipp8u)temp;

    /* store aspect ratio */
    jerr = _READ_WORD(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->jfif_app0_Xdensity = (Ipp16u)temp;

    jerr = _READ_WORD(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->jfif_app0_Ydensity = (Ipp16u)temp;

    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->JPGThumbWidth  = temp;

    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    jprops->JPGThumbHeight = temp;

    len -= 9;

    if(jprops->iotype == IJL_JFILE_READTHUMBNAIL ||
       jprops->iotype == IJL_JBUFF_READTHUMBNAIL)
    {
      if(len < jprops->JPGThumbWidth * jprops->JPGThumbHeight * 3)
      {
        jerr = IJL_ERR_DATA;
        goto Exit;
      }

      /*
      // there's enough space in the JFIF marker for the thumbnail bytes.
      // write the thumbnail data to the output DIB
      */

      if(jprops->DIBColor == IJL_RGB)
      {
        /*
        // DIB color format is RGB
        // if DIB height > 0, write thumbnail from the top down
        */

        for(i = 0; i < jprops->JPGThumbHeight; i++)
        {
          for(j = 0; j < jprops->JPGThumbWidth * 3; j += 3)
          {
            jerr = _READ_BYTE(pState,&t[0]);
            if(IJL_OK != jerr)
              goto Exit;

            jerr = _READ_BYTE(pState,&t[1]);
            if(IJL_OK != jerr)
              goto Exit;

            jerr = _READ_BYTE(pState,&t[2]);
            if(IJL_OK != jerr)
              goto Exit;

            *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 0) = (Ipp8u)t[0];
            *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 1) = (Ipp8u)t[1];
            *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 2) = (Ipp8u)t[2];

            len -= 3;
          }
        }
      }
      else
      {
        /*
        // DIB color format is BGR. Must flip the bytes.
        // if DIB height > 0, write thumbnail from the top down
        */

        for(i = 0; i < jprops->JPGThumbHeight; i++)
        {
          for(j = 0; j < jprops->JPGThumbWidth * 3; j += 3)
          {
            jerr = _READ_BYTE(pState,&t[0]);
            if(IJL_OK != jerr)
              goto Exit;

            jerr = _READ_BYTE(pState,&t[1]);
            if(IJL_OK != jerr)
              goto Exit;

            jerr = _READ_BYTE(pState,&t[2]);
            if(IJL_OK != jerr)
              goto Exit;

            *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 2) = (Ipp8u)t[0];
            *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 1) = (Ipp8u)t[1];
            *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 0) = (Ipp8u)t[2];

            len -= 3;
          }
        }
      }
    }

    /* skip up to the end */
    for(i = len; i > 0; i--)
    {
      jerr = _READ_BYTE(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;
    }

    goto Exit;
  } /* JFIF */

  if(id[0] == 0x4A &&
     id[1] == 0x46 &&
     id[2] == 0x58 &&
     id[3] == 0x58 &&
     id[4] == 0x00)
  {
    /* marker JFXX */
    int thumb_type;

    if(FALSE == jprops->needframe)
    {
      /* if a frame has been seen */
      goto Exit;
    }

    /* found an extension JFIF marker segment. */
    jerr = _READ_BYTE(pState,&thumb_type);
    if(IJL_OK != jerr)
      goto Exit;

    len -= 1;

    switch(thumb_type)
    {
    case JFXX_THUMBNAILS_JPEG:
      /* thumbnail coded in JPEG. Not Supported. */
      jerr = IJL_INTERNAL_ERROR;
      break;

    case JFXX_THUMBNAILS_GRAY:
    {
      /* thumbnail stored using 1 bytes/pixel. */
      jerr = _READ_BYTE(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      jprops->JPGThumbWidth = (Ipp8u)temp;

      jerr = _READ_BYTE(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      jprops->JPGThumbHeight = (Ipp8u)temp;

      len -= 2;

      if(jprops->iotype == IJL_JFILE_READTHUMBNAIL ||
         jprops->iotype == IJL_JBUFF_READTHUMBNAIL)
      {
        Ipp8u palette[256*3];

        if(len < 256*3 + jprops->JPGThumbWidth * jprops->JPGThumbHeight)
        {
          jerr = IJL_ERR_DATA;
          goto Exit;
        }

        /* read the palette. */
        for(i = 0; i < 256*3; i++)
        {
          jerr = _READ_BYTE(pState,&temp);
          if(IJL_OK != jerr)
            goto Exit;

          len -= 1;

          palette[i] = (Ipp8u)temp;
        }

        /*
        // read the data.
        // if DIB height > 0, write thumbnail from the top down
        */
        for(i = 0; i < jprops->JPGThumbHeight; i++)
        {
          Ipp8u r;
          Ipp8u g;
          Ipp8u b;

          for(j = 0; j < jprops->JPGThumbWidth*3; j += 3)
          {
            jerr = _READ_BYTE(pState,&t[0]);
            if(IJL_OK != jerr)
              goto Exit;

            len -= 1;

            r = palette[t[0] * 3 + 0];
            g = palette[t[0] * 3 + 1];
            b = palette[t[0] * 3 + 2];

            if(jprops->DIBColor == IJL_RGB)
            {
              /* RGB */
              jprops->DIBBytes[jprops->DIBLineBytes*i + j + 0] = r;
              jprops->DIBBytes[jprops->DIBLineBytes*i + j + 1] = g;
              jprops->DIBBytes[jprops->DIBLineBytes*i + j + 2] = b;
            }
            else
            {
              /* BGR */
              jprops->DIBBytes[jprops->DIBLineBytes*i + j + 0] = b;
              jprops->DIBBytes[jprops->DIBLineBytes*i + j + 1] = g;
              jprops->DIBBytes[jprops->DIBLineBytes*i + j + 2] = r;
            }
          }
        }
      }
    }
    break;

    case JFXX_THUMBNAILS_RGB:
    {
      /* thumbnail stored using 1 bytes/pixel. */
      jerr = _READ_BYTE(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      jprops->JPGThumbWidth = (Ipp8u)temp;

      jerr = _READ_BYTE(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;

      jprops->JPGThumbHeight = (Ipp8u)temp;

      len -= 2;

      if(jprops->iotype == IJL_JFILE_READTHUMBNAIL ||
         jprops->iotype == IJL_JBUFF_READTHUMBNAIL)
      {
        if(jprops->DIBColor == IJL_RGB)
        {
          /*
          // DIB color format is RGB
          // if DIB height > 0, write thumbnail from the top down
          */

          for(i = 0; i < jprops->JPGThumbHeight; i++)
          {
            for(j = 0; j < jprops->JPGThumbWidth * 3; j += 3)
            {
              jerr = _READ_BYTE(pState,&t[0]);
              if(IJL_OK != jerr)
                goto Exit;

              jerr = _READ_BYTE(pState,&t[1]);
              if(IJL_OK != jerr)
                goto Exit;

              jerr = _READ_BYTE(pState,&t[2]);
              if(IJL_OK != jerr)
                goto Exit;

              *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 0) = (Ipp8u)t[0];
              *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 1) = (Ipp8u)t[1];
              *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 2) = (Ipp8u)t[2];

              len -= 3;
            }
          }
        }
        else
        {
          /*
          // DIB color format is BGR. Must flip the bytes.
          // if DIB height > 0, write thumbnail from the top down
          */

          for(i = 0; i < jprops->JPGThumbHeight; i++)
          {
            for(j = 0; j < jprops->JPGThumbWidth * 3; j += 3)
            {
              jerr = _READ_BYTE(pState,&t[0]);
              if(IJL_OK != jerr)
                goto Exit;

              jerr = _READ_BYTE(pState,&t[1]);
              if(IJL_OK != jerr)
                goto Exit;

              jerr = _READ_BYTE(pState,&t[2]);
              if(IJL_OK != jerr)
                goto Exit;

              *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 2) = (Ipp8u)t[0];
              *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 1) = (Ipp8u)t[1];
              *(jprops->DIBBytes + (jprops->DIBLineBytes*i) + j + 0) = (Ipp8u)t[2];

              len -= 3;
            }
          }
        }
      }
    }
    break;
    } /* switch(thumb_type) */

    /* skip up to the end */
    for(i = len; i > 0; i--)
    {
      jerr = _READ_BYTE(pState,&temp);
      if(IJL_OK != jerr)
        goto Exit;
    }

    goto Exit;
  } /* JFXX */

Skip:

  /* skip up to the end */
  for(i = len; i > 0; i--)
  {
    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave DP_Parse_APP0\n");

  return IJL_OK;
} /* DP_Parse_APP0() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Parse_COM()
//
//  Purpose
//    Parse JFIF COM segment.
//
//  Context
//    Called from the main Decoder routine.
//
//  Returns
//    A valid error code, or 0 if no error
//
//  Parameters
//    pState       The pointer to current IJL state
//    pComment     The pointer to memory buffer to store comment
//    pCommentSize The pointer to variable to describe/store comment len
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_COM(
  STATE*  pState,
  Ipp8u*  pComment,
  Ipp16u* pCommentSize)
{
  int     i;
  int     len;
  int     byte;
  IJLERR  jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in DP_Parse_COM\n");

  TRY_BEGIN

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: Get_Segment_Length\n");
    goto Exit;
  }

  if(len < 2)
  {
    TRACE1(trERROR,"ERROR: len is %d\n",len);
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  len -= 2;

  if(NULL == pComment)
  {
    for(i = 0; i < len; i++)
    {
      jerr = _READ_BYTE(pState,&byte);
      if(IJL_OK != jerr)
        goto Exit;
    }
    goto Exit;
  }

  if(len > pCommentSize[0])
  {
    for(i = 0; i < pCommentSize[0]; i++)
    {
      jerr = _READ_BYTE(pState,&byte);
      if(IJL_OK != jerr)
        goto Exit;

      pComment[i] = (Ipp8u)byte;
    }

    pCommentSize[0] = (Ipp16u)len;

    jerr = IJL_ERR_COM_BUFFER;
  }
  else
  {
    for(i = 0; i < len; i++)
    {
      jerr = _READ_BYTE(pState,&byte);
      if(IJL_OK != jerr)
        goto Exit;

      pComment[i] = (Ipp8u)byte;
    }

    pCommentSize[0] = (Ipp16u)len;
  }

  TRY_END

  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: EXCEPTION detected in DP_Parse_COM()\n");
  jerr = IJL_ERR_COM_BUFFER;

  CATCH_END

Exit:

  TRACE0(trCALL|trAFTER,"leave DP_Parse_COM\n");

  return jerr;
} /* DP_Parse_COM() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Parse_DQT()
//
//  Purpose
//    This routine fills the IJL raw quantization tables upon seeing
//    a DQT segment. The Tables are then processed into a form usable
//    by the dequantizer.
//
//  Context
//    Called by Get_Quantization_Tables
//
//  Returns
//    a valid error code, or 0 if no error
//
//  Parameters
//    pState   The pointer to current IJL working state
//    jprops   The pointer to IJL Properties structure
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_DQT(
  STATE*           pState,
  JPEG_PROPERTIES* jprops)
{
  int    i;
  int    j;
  int    len;
  int    ident;
  int    quant;
  int    precision;
  Ipp8u   rawqtbl[DCTSIZE2];
  IJLERR jerr;

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
    goto Exit;

  len -= 2;

  /*
  // The maximum segment length allowed for a quantization table segment
  // corresponds to 4, 16-bit tables (129 bytes *4 tables, or 516 bytes).
  // If the detected segment length is > 516 bytes, this must indicate an
  // error condition and this routine will exit with an error.
  */

  if(len > 516 || len < 65)
  {
    TRACE1(trERROR,"ERROR: len is %d\n",len);
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  for(i = 0; i < len; )
  {
    jerr = _READ_BYTE(pState,&ident);
    if(IJL_OK != jerr)
      goto Exit;

    precision = (ident >> 4) & 0x0f;

    jprops->jFmtQuant[ident].precision = precision;

    ident &= 0x0f;

    if(ident < 0 || ident > 4)
    {
      TRACE1(trERROR,"ERROR: ident is %d\n",ident);
      jerr = IJL_BAD_QUANT_TABLE;
      goto Exit;
    }

    TRACE1(trINFO,"ident is %d\n",ident);

    i++;

    if(jprops->maxquantindex <= ident)
    {
      jprops->maxquantindex = (ident + 1);
    }

    if(precision == 0)
    {
      /* 8-bit quantiztion tables; */
      TRACE1(trINFO,"precision is %d [8-bit]\n",precision);

      for(j = 0; j < DCTSIZE2; j++)
      {
        jerr = _READ_BYTE(pState,&quant);
        if(IJL_OK != jerr)
          goto Exit;

        rawqtbl[j] = (Ipp8u)quant;
      }

      FillDecoderQuantTable(rawqtbl,jprops->jFmtQuant[ident].elements);

      i += DCTSIZE2;
    }
    else
    {
      /*
      // 16-bit quantization tables.
      // This will break as shorts are signed;
      // need jFmtQuant[].elements = unsigned.
      */
      TRACE1(trINFO,"precision is %d [16-bit]\n",precision);

      for(j = 0; j < DCTSIZE2; j++)
      {
        jerr = _READ_WORD(pState,&quant);
        if(IJL_OK != jerr)
          goto Exit;

        jprops->jFmtQuant[ident].elements[j] = (Ipp16u)quant;
      }

      IPPCALL(ippiZigzagInv8x8_16s_C1)(
        (const Ipp16s*)jprops->jFmtQuant[ident].elements,
              (Ipp16s*)jprops->jFmtQuant[ident].elements);

      i += DCTSIZE2*sizeof(Ipp16s);
    }

    /* store quant tables for user, if he wants */
    if(NULL != jprops->rawquanttables[ident].quantizer)
    {
      jprops->rawquanttables[ident].ident = (Ipp8u)ident;

      if(precision == 0)
      {
        for(j = 0; j < DCTSIZE2; j++)
        {
          jprops->rawquanttables[ident].quantizer[j] = rawqtbl[j];
        }
      }
      else
      {
        for(j = 0; j < DCTSIZE2*sizeof(Ipp16s); j++)
        {
          jprops->rawquanttables[ident].quantizer[j] =
            (Ipp8u)jprops->jFmtQuant[ident].elements[j];
        }
      }
    }

    if(jprops->nqtables < 4)
    {
      jprops->nqtables++;
    }
  } /* for len */

Exit:

  return jerr;
} /* DP_Parse_DQT() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Parse_DHT
//
//  Purpose
//    Fills the IJL raw huffman tables upon seeing a DHT segment.
//    the tables are then processed into a form usable by the huffman
//    decoder.
//
//  Context
//    Called from Get_Huffman_Tables
//
//  Returns
//    none
//
//  Parameters
//    pState      The pointer to current IJL working state
//    jprops      The pointer to IJL Properties structure
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_DHT(
  STATE*           pState,
  JPEG_PROPERTIES* jprops)
{
  int    i;
  int    len;
  int    val;
  int    sum;
  int    type;
  int    ident;
  Ipp8u  bits[16];
  Ipp8u  vals[256];
  IJLERR jerr;

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
    goto Exit;

  len -= 2;

  /*
  //  Maximal huffman table len is max_hlen = 1+16+256 = 273
  //  Maximal DHT len 4*max_hlen = 1092
  */

  if(len > 1092)
  {
    TRACE1(trERROR,"ERROR: length is %d\n",len);
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  while(len > 0)
  {
    jerr = _READ_BYTE(pState,&type);
    if(IJL_OK != jerr)
      goto Exit;

    ident = (type & 0x0f);
    type  = (type >> 4) & 0x0f;

    /* Check read huffman class (0 or 1, AC or DC) */
    if(type < 0 || type > 1)
    {
      TRACE1(trERROR,"ERROR: huff_class is %d\n",type);
      jerr = IJL_BAD_HUFFMAN_TABLE;
      goto Exit;
    }

    /* Check read table number (0 - 3) */
    if(ident < 0 || ident > 3)
    {
      TRACE1(trERROR,"ERROR: ident is %d\n",ident);
      jerr = IJL_BAD_HUFFMAN_TABLE;
      goto Exit;
    }

    if(jprops->maxhuffindex <= ident)
    {
      jprops->maxhuffindex = (ident + 1);
    }

    IPPCALL(ippsZero_8u)(bits,sizeof(bits));
    IPPCALL(ippsZero_8u)(vals,sizeof(vals));

    /* read list of bits */
    for(sum = 0, i = 0; i < 16; i++)
    {
      jerr = _READ_BYTE(pState,&val);
      if(IJL_OK != jerr)
        goto Exit;

      bits[i] = (Ipp8u)val;
      sum += val;
    }

    if(sum > 255)
    {
      jerr = IJL_BAD_HUFFMAN_TABLE;
      goto Exit;
    }

    /* read list of vals */
    for(i = 0; i < sum; i++)
    {
      jerr = _READ_BYTE(pState,&val);
      if(IJL_OK != jerr)
        goto Exit;

      vals[i] = (Ipp8u)val;
    }

    if(type)
    {
      /* AC Huffman Table */
      TRACE1(trINFO,"type  = %d [AC table]\n",type);
      TRACE1(trINFO,"ident = %d\n",ident);

      jprops->jFmtAcHuffman[ident].ident = ident;
      jprops->jFmtAcHuffman[ident].huff_class = type;

      jerr = BuildDecoderHuffmanTable(
        bits,
        vals,
        &jprops->jFmtAcHuffman[ident]);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: BuildDecoderHuffmanTable() failed\n");
        jerr = IJL_BAD_HUFFMAN_TABLE;
        goto Exit;
      }

      if(jprops->nhuffActables < 4)
      {
        jprops->nhuffActables++;
      }
    }
    else
    {
      /* DC Huffman Table */
      TRACE1(trINFO,"type  = %d [DC table]\n",type);
      TRACE1(trINFO,"ident = %d\n",ident);

      jprops->jFmtDcHuffman[ident].ident = ident;
      jprops->jFmtDcHuffman[ident].huff_class = type;

      jerr = BuildDecoderHuffmanTable(
        bits,
        vals,
        &(jprops->jFmtDcHuffman[ident]));

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: BuildDecoderHuffmanTable() failed\n");
        jerr = IJL_BAD_HUFFMAN_TABLE;
        goto Exit;
      }

      if(jprops->nhuffDctables < 4)
      {
        jprops->nhuffDctables++;
      }
    }

    /* store raw tables for user, if he wants */
    if(NULL != jprops->rawhufftables[ident].bits &&
       NULL != jprops->rawhufftables[ident].vals)
    {
      jprops->rawhufftables[ident*2 + type].ident = (Ipp8u)ident;
      jprops->rawhufftables[ident*2 + type].hclass = (Ipp8u)type;

      /* read list of bits */
      for(i = 0; i < 16; i++)
      {
        jprops->rawhufftables[ident*2 + type].bits[i] = bits[i];
      }

      /* read list of vals */
      for(i = 0; i < sum; i++)
      {
        jprops->rawhufftables[ident*2 + type].vals[i] = vals[i];
      }
    }

    len -= sum + 16 + 1;
  } /* while len */

Exit:

  return jerr;
} /* DP_Parse_DHT() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_Parse_SOS
//
//  Purpose
//    After seeing a SOS marker, this routine reads the SOS segment
//    and fills a scan structure with information relevant to decoding
//    this scan.
//
//  Context
//    Called from Get_Scan
//
//  Returns
//    a valid error code or
//
//  Parameters
//    pState   The pointer to current IJL working state
//    pFrame   The pointer to current IJL frame structure
//    pScan    The pointer to scan structure to fill
//    jprops   The pointer to IJL Properties structure
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_Parse_SOS(
  STATE*           pState,
  FRAME*           pFrame,
  SCAN*            pScan,
  JPEG_PROPERTIES* jprops)
{
  int             i;
  int             j;
  int             len;
  int             temp;
  int             comp_sel;
  int             dctab_sel;
  int             actab_sel;
  IJL_CONTEXT*    ctx  = (IJL_CONTEXT*)pState->ctx;
  SCAN_COMPONENT* pScanComp = NULL;
  IJLERR          jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in DP_Parse_SOS()\n");

  TRY_BEGIN

  jerr = _READ_WORD(pState,&len);
  if(IJL_OK != jerr)
  {
    goto Exit;
  }

  if(len < 2)
  {
    TRACE1(trERROR,"ERROR: invalid segment length: %d\n",len);
    jerr = IJL_ERR_DATA;
    goto Exit;
  }

  /* SCAN segment length + SOS marker length */
  ctx->scan_length = len + 2;

  /* exclude length field length */
  len -= 2;

  jerr = _READ_BYTE(pState,&temp);
  if(IJL_OK != jerr)
    goto Exit;

  pScan->ncomps = temp;

  pScan->comps = ippMalloc(sizeof(SCAN_COMPONENT)*pScan->ncomps);
  if(NULL == pScan->comps)
  {
    TRACE1(trERROR,"ERROR: can't allocate %d bytes for SCAN_COMPONENTs\n",
      sizeof(SCAN_COMPONENT)*pScan->ncomps);
    jerr = IJL_MEMORY_ERROR;
    goto Exit;
  }

  TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for SCAN_COMPONENTs\n",
    sizeof(SCAN_COMPONENT)*pScan->ncomps,pScan->comps);

  for(pScanComp = pScan->comps, i = 0; i < pScan->ncomps; i++, pScanComp++)
  {
    jerr = _READ_BYTE(pState,&comp_sel);
    if(IJL_OK != jerr)
      goto Exit;

    jerr = _READ_BYTE(pState,&temp);
    if(IJL_OK != jerr)
      goto Exit;

    dctab_sel = temp >> 4;
    actab_sel = temp & 0x0f;

    if(dctab_sel < 0 || dctab_sel > 4 ||
       actab_sel < 0 || actab_sel > 4)
    {
      TRACE0(trERROR,"ERROR: invalid table selector\n");
      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENT's\n",
        pScan->comps);
      ippFree(pScan->comps);
      pScan->comps = NULL;
      jerr = IJL_ERR_DATA;
      goto Exit;
    }

    for(j = 0; j < pFrame->ncomps; j++)
    {
      /* Search component ident */
      if(comp_sel == pFrame->comps[j].ident)
      {
        break;
      }
    }

    /* Search Failed or out of range */
    if(pFrame->ncomps <= j)
    {
      TRACE0(trERROR,"ERROR: can't find frame component\n");
      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENTs\n",
        pScan->comps);
      ippFree(pScan->comps);
      pScan->comps = NULL;
      jerr = IJL_ERR_DATA;
      goto Exit;
    }
    else
    {
      pScanComp->comp        = j;
      pScanComp->hsampling   = pFrame->comps[j].hsampling;
      pScanComp->vsampling   = pFrame->comps[j].vsampling;
      pScanComp->dc_table    = &jprops->jFmtDcHuffman[dctab_sel];
      pScanComp->ac_table    = &jprops->jFmtAcHuffman[actab_sel];
      pScanComp->quant_table = &jprops->jFmtQuant[pFrame->comps[j].quant_sel];

      jprops->HuffIdentifierDC[i] = dctab_sel;
      jprops->HuffIdentifierAC[i] = actab_sel;

      TRACE1(trINFO,"comp->comp        = %d\n",pScanComp->comp);
      TRACE1(trINFO,"comp->hsampling   = %d\n",pScanComp->hsampling);
      TRACE1(trINFO,"comp->vsampling   = %d\n",pScanComp->vsampling);
      TRACE1(trINFO,"comp->dc_table    = %x\n",pScanComp->dc_table);
      TRACE1(trINFO,"comp->ac_table    = %x\n",pScanComp->ac_table);
      TRACE1(trINFO,"comp->quant_table = %x\n",pScanComp->quant_table);
    }
  }

  jerr = _READ_BYTE(pState,&pScan->start_spec);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _READ_BYTE(pState,&pScan->end_spec);
  if(IJL_OK != jerr)
    goto Exit;

  jerr = _READ_BYTE(pState,&temp);
  if(IJL_OK != jerr)
    goto Exit;

  pScan->approx_high = temp >> 4;
  pScan->approx_low  = temp & 0x0f;

  TRACE1(trINFO,"scan->start_spec  = %d\n",pScan->start_spec);
  TRACE1(trINFO,"scan->end_spec    = %d\n",pScan->end_spec);
  TRACE1(trINFO,"scan->approx_high = %d\n",pScan->approx_high);
  TRACE1(trINFO,"scan->approx_low  = %d\n",pScan->approx_low);

  TRY_END

  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: EXCEPTION detected in DP_Parse_SOS()\n");
  TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENTs\n",
    pScan->comps);
  ippFree(pScan->comps);
  pScan->comps = NULL;
  jerr = IJL_EXCEPTION_DETECTED;

  CATCH_END

Exit:

  TRACE0(trCALL|trAFTER,"leave DP_Parse_SOS()\n");

  return jerr;
} /* DP_Parse_SOS() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Get_Scan
//
//  Purpose
//    Gets the scan header information and checks to verify that
//    the entropy tables have been defined (as the following routines
//    will require them)
//
//  Context
//    Called before processing image data.
//
//  Returns
//    valid error code or 0 if OK
//
//  Parameters
//    pState   IJL state variables (technically a member
//             of JPEG_PROPERTIES)
//    pScan    scan variables of current scan.
//    jprops   IJL properties persistent storage.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Get_Scan(
  STATE*           pState,
  SCAN*            pScan,
  JPEG_PROPERTIES* jprops)
{
  int    i;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in Get_Scan()\n");

  if(jprops->needframe != 0)
  {
    TRACE0(trERROR,"ERROR: no frame found\n");
    jerr = IJL_ERR_NO_FRAME;
    goto Exit;
  }

  jerr = DP_Parse_SOS(pState,&jprops->jframe,pScan,jprops);

  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: wrong Start Of Scan segment\n");
    goto Exit;
  }

  IPPCALL(ippiDecodeHuffmanStateInit_JPEG_8u)(pState->u.pDecHuffState);

  /* If the tables are not defined, we can't process the image data. */
  for(i = 0; i < pScan->ncomps; i++)
  {
    if(NULL == pScan->comps[i].dc_table || NULL == pScan->comps[i].ac_table)
    {
      TRACE1(trERROR,"ERROR: comps [%d] has not huffman tables\n",i);
      jerr = IJL_ERR_NO_HUF;
      goto Exit;
    }

    if(NULL == pScan->comps[i].quant_table)
    {
      TRACE1(trERROR,"ERROR: comps [%d] has not quant tables\n",i);
      jerr = IJL_ERR_NO_QUAN;
      goto Exit;
    }
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave Get_Scan()\n");

  return jerr;
} /* Get_Scan() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    DP_ParseBitstream
//
//  Purpose
//
//  Context
//
//  Returns
//    a valid error code or
//
//  Parameters
//    jprops   The pointer to IJL Properties structure
//
///////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) DP_ParseBitstream(
  JPEG_PROPERTIES* jprops)
{
  int          i;
  BOOL         noreload_scan = FALSE;
  IJL_MARKER   marker        = MARKER_NONE;
  IJLERR       jerr          = IJL_OK;
  SCAN*        scan          = NULL;
  STATE*       state         = &jprops->state;
  IJL_CONTEXT* ctx           = (IJL_CONTEXT*)state->ctx;

  if(jprops->jscan != NULL)
  {
    noreload_scan = TRUE;
  }

  if(jprops->progressive_found == 0)
  {
    jprops->jframe.SeenAllACScans = 0;
    jprops->jframe.SeenAllDCScans = 0;
  }

  if((jprops->jscan != NULL) && ((jprops->jinterleaveType != 1) ||
     ((jprops->jinterleaveType == 1) &&
     ((jprops->jscan->curxMCU != 0) || (jprops->jscan->curyMCU != 0)))))
  {
    goto Got_Scan;
  }

  /* if the interleaveType flag is set, seek to the start of the      */
  /* image (required to get any Huffman tables defined between scans) */

  for(;;)
  {
ParseAgain:

    if(jprops->jscan != NULL && jprops->interrupt)
    {
      goto Got_Scan;
    }

    jerr = DP_Get_Next_Marker(state, &marker);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: read next marker failed\n");
      goto Exit;
    }

    if(marker == MARKER_EOF)
    {
      goto Exit;
    }

    switch(marker)
    {
      case MARKER_SOI:
      {
        TRACE0(trINFO,"marker Start of Image[SOI]\n");
        jprops->jframe.restart_interv = 0;
        break;
      }

      case MARKER_EOI:
      {
        TRACE0(trINFO,"marker End of Image[EOI]\n");

        if((jprops->iotype == IJL_JBUFF_READHEADER) ||
           (jprops->iotype == IJL_JFILE_READHEADER) ||
           (jprops->iotype == IJL_JBUFF_READPARAMS) ||
           (jprops->iotype == IJL_JFILE_READPARAMS) ||
           (jprops->iotype == IJL_JBUFF_READTHUMBNAIL) ||
           (jprops->iotype == IJL_JFILE_READTHUMBNAIL))
        {
          if(jprops->JPGWidth == 0 || jprops->JPGHeight == 0 || jprops->JPGChannels == 0)
          {
            jerr = IJL_ERR_DATA;
          }
          goto Exit;
        }
        else
        {
          if(jprops->jinterleaveType   != 0 &&
             jprops->cconversion_reqd  != 0 &&
             jprops->progressive_found == 0)
          {
            CConvert_Image(jprops);
          }
          goto Exit;
        }
      }

      case MARKER_SOF0:
      case MARKER_SOF1:
      case MARKER_SOF2:
      {
        TRACE0(trINFO,"marker Start of frame [SOF0]\n");

        if(jprops->iotype == IJL_JBUFF_READHEADER ||
           jprops->iotype == IJL_JFILE_READHEADER)
        {
          /* ignore the last two bytes we found (the MARKER_SOF0 marker) */
          state->entropy_bytes_processed -= 2;
          goto Exit;
        }

        jerr = DP_Parse_SOF(state,jprops);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: DP_Parse_SOF() failed\n");
          goto Exit;
        }

        /* We have captured the frame information. Do not zero frame until the */
        /* User issues an Init().                                              */
        jprops->needframe = 0;

        /* Support up to 4 components per scan */
        if(jprops->jframe.ncomps < 1 || jprops->jframe.ncomps > 255)
        {
          TRACE0(trERROR,"ERROR: invalid frame # comps\n");
          jerr = IJL_ERR_COMP;
          goto Exit;
        }

        /* This will clue the user if the image has lots of components */
        /* Image channels                                              */
        jprops->JPGChannels = jprops->jframe.ncomps;

        /* Image sizes */
        jprops->JPGWidth  = jprops->jframe.width;
        jprops->JPGHeight = jprops->jframe.height;

        jerr = Set_Decode_Fast_Path(jprops);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: Set_Decode_Fast_Path() failed\n");
          goto Exit;
        }

        break;
      }

      case MARKER_SOF3:
      case MARKER_SOF5:
      case MARKER_SOF6:
      case MARKER_SOF7:
      case MARKER_SOF8:
      case MARKER_SOF9:
      case MARKER_SOFA:
      case MARKER_SOFB:
      case MARKER_SOFD:
      case MARKER_SOFE:
      case MARKER_SOFF:
        jerr = IJL_UNSUPPORTED_FRAME;
        goto Exit;

      case MARKER_SOS:
      {
        TRACE0(trINFO,"marker Start of Scan[SOS]\n");
Got_Scan:
        /* Scan is allocated here. Scan goes out of scope if a restart or */
        /* interrupt is detected. Thus all scan-specific members must be  */
        /* persisted. */

        if(NULL != jprops->jscan && 0 != noreload_scan)
        {
          scan          = jprops->jscan;
          noreload_scan = FALSE;
        }
        else
        {
          scan = ippMalloc(sizeof(SCAN));

          if(scan == NULL)
          {
            TRACE0(trERROR,"ERROR: not enough memory\n");
            jerr = IJL_MEMORY_ERROR;
            goto Exit;
          }

          TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for SCAN struct\n",sizeof(SCAN),scan);

          scan->curxMCU    = 0;
          scan->curyMCU    = 0;
          scan->dc_diff[0] = 0;
          scan->dc_diff[1] = 0;
          scan->dc_diff[2] = 0;
          scan->dc_diff[3] = 0;

          jerr = Get_Scan(state, scan, jprops);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: Get_Scan() failed\n");
            TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN struct\n",scan);
            ippFree(scan->comps);
            scan->comps = NULL;
            ippFree(scan);
            scan = NULL;
            goto Exit;
          }

          if((jprops->iotype == IJL_JBUFF_READPARAMS) ||
             (jprops->iotype == IJL_JFILE_READPARAMS) ||
             (jprops->iotype == IJL_JBUFF_READHEADER) ||
             (jprops->iotype == IJL_JFILE_READHEADER) ||
             /* pass_num #1 exits at SOF or EOI */
             (jprops->iotype == IJL_JBUFF_READTHUMBNAIL) ||
             (jprops->iotype == IJL_JFILE_READTHUMBNAIL))
          {
            state->entropy_bytes_processed -= ctx->scan_length;
            TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN struct\n",scan);
            ippFree(scan->comps);
            scan->comps = NULL;
            ippFree(scan);
            scan = NULL;
            goto Exit;
          }

          scan->restart_interv = jprops->jframe.restart_interv;

          /* Adjust interleave for multi-scan images. */
          if((scan->ncomps < jprops->jframe.ncomps) && (jprops->progressive_found == 0))
          {
            jprops->jinterleaveType = 1;
          }
          else
          {
            jprops->jinterleaveType = 0;
          }

          Set_Decode_Fast_Path(jprops);

          if((scan->start_spec  != 0) || (scan->end_spec   != 63) ||
             (scan->approx_high != 0) || (scan->approx_low != 0)  ||
             (jprops->progressive_found != 0))
          {
            /* is this a DC or an AC scan? */
            if((scan->start_spec == 0) && (scan->end_spec == 0))
            {
              /* a DC scan
              // increment the DC scan counter. This counter will be used
              // for modes (like scaled decoding) that need to know
              // when all of the DC component have been specified.
              */

              TRACE0(trINFO,"found DC scan\n");

              if(scan->approx_low == 0)
              {
                jprops->jframe.SeenAllDCScans += scan->ncomps;
              }
            }
            else
            {
              /* an AC scan
              // similar to the AC case.
              // there is always only one component per scan in the
              // progressive AC case.
              */

              TRACE0(trINFO,"found AC scan\n");

              if(scan->approx_low == 0 && scan->end_spec == 63)
              {
                jprops->jframe.SeenAllACScans++;
              }
            }

            jprops->progressive_found++;

            /*
            // uh-oh. Just found a progressive image. Allocate enough space
            // for an image-sized coefficient buffer.
            // determine the number of blocks per MCU.
            */

            if(jprops->progressive_found == 1)
            {
              int buf_size;
              int size = 0;

              for(i = 0; i < jprops->jframe.ncomps; i++)
              {
                size += (jprops->jframe.comps[i].hsampling *
                         jprops->jframe.comps[i].vsampling);
              }

              buf_size = jprops->numxMCUs * jprops->numyMCUs * size * 64;

              jprops->coef_buffer = ippMalloc(sizeof(Ipp16s)*buf_size);

              if(NULL == jprops->coef_buffer)
              {
                TRACE1(trERROR,"ERROR: can't allocate %d bytes for coef_buf\n",sizeof(short)*buf_size);

                TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENTs\n",scan->comps);
                ippFree(scan->comps);
                scan->comps = NULL;

                TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN struct\n",scan);
                ippFree(scan);

                scan = NULL;
                jprops->jscan = NULL;

                jerr = IJL_MEMORY_ERROR;
                goto Exit;
              }
              TRACE2(
                trMEMORY,
                "allocate %d bytes at 0x%08X for coefficient buffer\n",
                buf_size,
                jprops->coef_buffer);
            }
          }
        }

        if((jprops->jinterleaveType == 1) && (jprops->progressive_found == 0))
        {
          /*
          // increment the scan counter.
          // Will be >=jprops->jframe.ncomps when on the last scan.
          */
          jprops->jframe.SeenAllACScans += scan->ncomps;
        }

        jprops->jscan = scan;

        /* 1-row buffer for qualitative up-sampling */
        if(IJL_TRIANGLE_FILTER == jprops->upsampling_type)
        {
          int size = 0;
          for(i = 0; i < jprops->jframe.ncomps; i++)
          {
            size += (jprops->jframe.comps[i].hsampling *
                     jprops->jframe.comps[i].vsampling);
          }

          size *= (jprops->numxMCUs + 2) * DCTSIZE2;

          if(NULL == jprops->sampling_state_ptr)
          {
            jprops->sampling_state_ptr = ippMalloc(sizeof(SAMPLING_STATE));

            if(NULL == jprops->sampling_state_ptr)
            {
              TRACE0(trERROR,"ERROR: not enough memory\n");
              jerr = IJL_MEMORY_ERROR;
              goto Exit;
            }

            TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for SAMPLING_STATE struct\n",
              sizeof(SAMPLING_STATE),jprops->sampling_state_ptr);

            jprops->sampling_state_ptr->top_row = ippMalloc(sizeof(Ipp16s) * size * 4);

            if(NULL == jprops->sampling_state_ptr->top_row)
            {
              TRACE0(trERROR,"ERROR: not enough memory\n");
              TRACE1(trMEMORY,"freeing memory at 0x%08X for SAMPLING_STATE struct\n",jprops->sampling_state_ptr);
              ippFree(jprops->sampling_state_ptr);
              jprops->sampling_state_ptr = NULL;

              jerr = IJL_MEMORY_ERROR;
              goto Exit;
            }
            jprops->sampling_state_ptr->cur_row    = jprops->sampling_state_ptr->top_row    + size;
            jprops->sampling_state_ptr->bottom_row = jprops->sampling_state_ptr->cur_row    + size;
            jprops->sampling_state_ptr->last_row   = jprops->sampling_state_ptr->bottom_row + size;
            jprops->sampling_state_ptr->cur_row_number = 0;
          }
        }

        /* Decode the scan data. */
        jerr = Decode_Scan(&marker,state, scan, jprops);

        /* return without freeing the scan pointer if an interrupt was detected. */
        if((IJL_INTERRUPT_OK == jerr) || (IJL_ROI_OK == jerr))
        {
          /*
          // if in progressive mode, and we get here, we have read the
          // entire image into the MCU Buffer. Update a flag to indicate
          // we shouldn't try to decode any more entropy data.
          */

          if(jprops->progressive_found != 0 && jprops->interrupt == 0)
          {
            jprops->jframe.SeenAllACScans++;

            jprops->jscan     = scan;
            jprops->interrupt = FALSE;
            goto Exit;
          }
          else
          {
            /* perform color conversion on the multiscan data, if necessary. */
            if((jprops->jinterleaveType == 1) && (jerr == IJL_ROI_OK))
            {
              if(jprops->cconversion_reqd)
              {
                CConvert_Image(jprops);
              }

              /*
              // if decoding a ROI of a multiscan image and no interrupt was
              // found, we need to clear the scan persistence because on
              // subsequent entry into the IJL we will need to re-load the
              // per-scan information.
              */
              TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENTs\n",scan->comps);
              ippFree(scan->comps);
              scan->comps = NULL;

              TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN struct\n",scan);
              ippFree(scan);
              scan          = NULL;
              jprops->jscan = NULL;

              goto ParseAgain;
            }
            else
            {
              jprops->jscan     = scan;
              jprops->interrupt = FALSE;
              goto Exit;
            }
          }
        } /* if interrupt or ROI */

        /*
        // Advance the scan counter to the next component.
        // This counter is used in non-progressive mode decoding for dealing
        // with images that have non-interleaved block orders.
        */
        if(jprops->progressive_found == 0)
        {
          state->cur_scan_comp += scan->ncomps;
        }

        /* Free the scan structure and scan components structures */
        if(NULL != jprops->jscan && IJL_OK == jerr)
        {
          TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENTs\n",jprops->jscan->comps);
          ippFree(jprops->jscan->comps);
          jprops->jscan->comps = NULL;

          TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN struct\n",jprops->jscan);
          ippFree(jprops->jscan);
          jprops->jscan = NULL;
        }

        if(IJL_OK > jerr)
        {
          goto Exit;
        }

        if(IJL_ROI_OK == jerr || IJL_INTERRUPT_OK == jerr)
        {
          goto Exit;
        }

        break;
      }

      case MARKER_DHT:
      {
        TRACE0(trINFO,"marker Define Huffman Tables [DHT]\n");

        jerr = DP_Parse_DHT(state,jprops);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: DP_Parse_DHT() failed\n");
          goto Exit;
        }

        break;
      }

      case MARKER_RST0:
      case MARKER_RST1:
      case MARKER_RST2:
      case MARKER_RST3:
      case MARKER_RST4:
      case MARKER_RST5:
      case MARKER_RST6:
      case MARKER_RST7:
        break;

      case MARKER_DQT:
      {
        TRACE0(trINFO,"marker Define Quant Tables[DQT]\n");

        jerr = DP_Parse_DQT(state,jprops);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: DP_Parse_DQT() failed\n");
          goto Exit;
        }

        break;
      }

      case MARKER_DRI:
      {
        int restart;

        TRACE0(trINFO,"marker Define Restrt Interval [DRI]\n");

        jerr = DP_Parse_DRI(state, &restart);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: DP_Parse_DRI() failed\n");
          goto Exit;
        }

        TRACE1(trINFO,"restart interval is %d\n",restart);

        jprops->jframe.restart_interv = restart;

        break;
      }

      case MARKER_APP0:
      {
        TRACE0(trINFO,"marker Application segment 0[APP0]\n");

        /* shouldn't see an application marker if decoding entropy data. */
        if(IJL_JFILE_READENTROPY == jprops->iotype)
        {
          break;
        }

        jerr = DP_Parse_APP0(state,jprops);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: DP_Parse_APP0() failed\n");
          goto Exit;
        }

        break;
      }

      case MARKER_APP14:
      {
        TRACE0(trINFO,"marker Application segment 14[APP14]\n");

        jerr = DP_Parse_APP14(
          &jprops->SawAdobeMarker,
          &jprops->AdobeVersion,
          &jprops->AdobeFlags0,
          &jprops->AdobeFlags1,
          &jprops->AdobeXform,
          state);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: DP_Parse_APP14() failed\n");
          goto Exit;
        }

        if(jprops->needframe == 0)
        {
          jerr = Set_Decode_Fast_Path(jprops);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: Set_Decode_Fast_Path() failed\n");
            goto Exit;
          }
        }

        break;
      }

      case MARKER_EOF:
      {
        TRACE0(trINFO,"the End Of File reached\n");
        break;
      }

      case MARKER_COM:
        TRACE0(trINFO,"marker Comment[COM]\n");
        if(FALSE != jprops->needframe)
        {
          jerr = DP_Parse_COM(
            state,
            (Ipp8u*)jprops->jpeg_comment,
            &jprops->jpeg_comment_size);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: DP_Parse_COM() failed\n");
            goto Exit;
          }
        }
        else
        {
          jerr = DP_Skip(state);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: DP_Skip() failed\n");
            goto Exit;
          }
        }
        break;

      case MARKER_NONE:
      case MARKER_DNL:
      case MARKER_DHP:
      case MARKER_EXP:
      case MARKER_DAC:
      case MARKER_APP1:
      case MARKER_APP2:
      case MARKER_APP3:
      case MARKER_APP4:
      case MARKER_APP5:
      case MARKER_APP6:
      case MARKER_APP7:
      case MARKER_APP8:
      case MARKER_APP9:
      case MARKER_APP10:
      case MARKER_APP11:
      case MARKER_APP12:
      case MARKER_APP13:
      case MARKER_JPG1:
      case MARKER_JPG2:
      case MARKER_JPG3:
      case MARKER_JPG4:
      case MARKER_JPG5:
      case MARKER_JPG6:
      case MARKER_JPG7:
      case MARKER_JPG8:
      case MARKER_JPG9:
      case MARKER_JPG10:
      case MARKER_JPG11:
      case MARKER_JPG12:
      case MARKER_JPG14:
      default:
      {
        TRACE0(trINFO,"skipping...\n");

        jerr = DP_Skip(state);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: DP_Skip() failed\n");
          goto Exit;
        }
        break;
      }
    }
  } /* parser loop */

Exit:

  return jerr;
} /* DP_ParseBitstream() */

