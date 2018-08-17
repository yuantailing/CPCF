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
#ifndef __FRAME_ENCODER_H__
#include "frame_encoder.h"
#endif



#ifdef __INTEL_COMPILER
#pragma warning(disable:171)
#pragma warning(disable:424)
#endif


EXC_INIT();


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    ownEncoderWriteJPEGTables
//
//  Purpose
//    Write entropy tables to a JPEG stream.
//    This function writes quantization and Huffman tables,
//    using pre-defined tables if none are supplied by
//    the user. Then it builds an encoder-usable
//    version of the tables and stores that version in
//    JPEG_PROPERTIES. This function is not meant to be
//    called frequently, because the function
//    BuildEncoderHuffmanTable() is kind of slow.
//
//  Context
//
//  Returns
//    Valid error code, or 0 for OK.
//
//  Parameters
//    jprops   Pointer to global IJL properties storage.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) ownEncoderWriteJPEGTables(
  JPEG_PROPERTIES* jprops)
{
  int    i;
  STATE* pState = &jprops->state;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in ownEncoderWriteJPEGTables()\n");

  /* /////////////////////////////////////////////////////////////////////////
  // Quantization Tables
  */

  for(i = 0; i < jprops->maxquantindex; i++)
  {
    Ipp8u rawqtbl[DCTSIZE2];

    Scale_Char_Matrix(jprops->rawquanttables[i].quantizer,jprops->jquality,rawqtbl);

    jprops->jEncFmtQuant[i] = jprops->jFmtQuant[i].elements;

    FillEncoderQuantTable(rawqtbl,jprops->jEncFmtQuant[i]);

    jerr = EP_Write_DQT(0,jprops->rawquanttables[i].ident,rawqtbl,pState);
    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed EP_Write_DQT()\n");
      goto Exit;
    }
  }

  if(jprops->progressive_found == 0)
  {
    /* ///////////////////////////////////////////////////////////////////////
    // Huffman Tables
    */

    jerr = EP_Write_DHTs(jprops,pState);
    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed EP_Write_DHT()\n");
      goto Exit;
    }

    /* The Huffman Tables alternate as: DC-AC-DC-AC-... */
    for(i = 0; i < jprops->maxhuffindex * 2; i += 2)
    {
      jprops->jEncFmtDcHuffman[jprops->rawhufftables[i  ].ident] = &(jprops->jFmtDcHuffman[i]);
      jprops->jEncFmtAcHuffman[jprops->rawhufftables[i+1].ident] = &(jprops->jFmtAcHuffman[i]);

      jerr = BuildEncoderHuffmanTable(
        jprops->rawhufftables[i].bits,
        jprops->rawhufftables[i].vals,
        jprops->jEncFmtDcHuffman[jprops->rawhufftables[i].ident]);
      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
        goto Exit;
      }

      jerr = BuildEncoderHuffmanTable(
        jprops->rawhufftables[i+1].bits,
        jprops->rawhufftables[i+1].vals,
        jprops->jEncFmtAcHuffman[jprops->rawhufftables[i+1].ident]);
      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
        goto Exit;
      }
    }
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave ownEncoderWriteJPEGTables()\n");

  return jerr;
} /* ownEncoderWriteJPEGTables() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Encode_Frame_Init
//
//  Purpose
//    Writes partial "header" information to the file.
//    This "header" information includes SOI, APP0,
//    tables (DHT, DQT) and EOI data segments where
//    appropriate.
//
//  Context
//
//  Returns
//    Valid error code, or 0 for OK.
//
//  Parameters
//    jprops   Pointer to global IJL properties storage.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Encode_Frame_Init(
  JPEG_PROPERTIES* jprops)
{
  int i;
  STATE* pState = &jprops->state;
  IJLERR jerr  = IJL_OK;
  IppStatus status;

  TRACE0(trCALL|trBEFORE,"enter in Encode_Frame_Init()\n");

  status = IPPCALL(ippiEncodeHuffmanStateInit_JPEG_8u)(pState->u.pEncHuffState);
  if(ippStsNoErr != status)
  {
    return IJL_MEMORY_ERROR;
  }

  EB_Init(jprops->JPGBytes,jprops->JPGSizeBytes,pState);

  /* start of image */
  jerr = EP_Write_SOI(pState);

  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: failed EP_Write_SOI()\n");
    goto Exit;
  }

  if(jprops->iotype == IJL_JFILE_WRITEWHOLEIMAGE ||
     jprops->iotype == IJL_JBUFF_WRITEWHOLEIMAGE ||
     jprops->iotype == IJL_JFILE_WRITEHEADER     ||
     jprops->iotype == IJL_JBUFF_WRITEHEADER)
  {
    if(jprops->JPGColor == IJL_RGB || jprops->JPGColor == IJL_RGBA_FPX)
    {
      jerr = EP_Write_APP14(
        jprops->AdobeVersion,
        0,
        jprops->AdobeFlags0,
        jprops->AdobeFlags1,
        pState);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed EP_Write_APP14(0,...)\n");
        goto Exit;
      }
    }
    else if(jprops->JPGColor == IJL_YCBCRA_FPX)
    {
      jerr = EP_Write_APP14(
        jprops->AdobeVersion,
        2,
        jprops->AdobeFlags0,
        jprops->AdobeFlags1,
        pState);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed EP_Write_APP14(2,...)\n");
        goto Exit;
      }
    }
    else
    {
      jerr = EP_Write_APP0(
        jprops->jfif_app0_version,
        jprops->jfif_app0_units,
        jprops->jfif_app0_Xdensity,
        jprops->jfif_app0_Ydensity,
        0,
        jprops->JPGThumbWidth,
        jprops->JPGThumbHeight,
        pState);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed EP_Write_APP0()\n");
        goto Exit;
      }
    }

    if(NULL == jprops->jpeg_comment)
    {
      Ipp8u  comment[80];
      int    comment_size;
      char*  fmt = "Intel(R) JPEG Library, version %s\0";
      const  IJLibVersion* version = ownGetLibVersion();

      comment_size = sprintf((char*)comment,fmt,version->InternalVersion);

      jerr = EP_Write_COM(
        comment,
        comment_size + 1,
        pState);
    }
    else
    {
      jerr = EP_Write_COM(
        (Ipp8u*)jprops->jpeg_comment,
        jprops->jpeg_comment_size,
        pState);
    }

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed EP_Write_COM()\n");
      goto Exit;
    }

    jerr = ownEncoderWriteJPEGTables(jprops);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed ownEncoderWriteJPEGTables()\n");
      goto Exit;
    }
  }

  if(jprops->iotype == IJL_JFILE_WRITEENTROPY ||
     jprops->iotype == IJL_JBUFF_WRITEENTROPY)
  {
    /* set default quant tables if none */
    for(i = 0; i < jprops->maxquantindex; i++)
    {
      if(NULL == jprops->jEncFmtQuant[i])
      {
        if(NULL != jprops->rawquanttables[i].quantizer)
        {
          Ipp8u rawqtbl[DCTSIZE2];
          Scale_Char_Matrix(jprops->rawquanttables[i].quantizer,jprops->jquality,rawqtbl);
          FillEncoderQuantTable(rawqtbl,jprops->jEncFmtQuant[i]);
        }
      }
    }

    /* set default huff tables if none */
    for(i = 0; i < jprops->maxhuffindex * 2; i += 2)
    {
      jprops->jEncFmtDcHuffman[jprops->rawhufftables[i  ].ident] = &(jprops->jFmtDcHuffman[i]);
      jprops->jEncFmtAcHuffman[jprops->rawhufftables[i+1].ident] = &(jprops->jFmtAcHuffman[i]);

      jerr = BuildEncoderHuffmanTable(
        jprops->rawhufftables[i].bits,
        jprops->rawhufftables[i].vals,
        jprops->jEncFmtDcHuffman[jprops->rawhufftables[i].ident]);
      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
        goto Exit;
      }

      jerr = BuildEncoderHuffmanTable(
        jprops->rawhufftables[i+1].bits,
        jprops->rawhufftables[i+1].vals,
        jprops->jEncFmtAcHuffman[jprops->rawhufftables[i+1].ident]);
      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
        goto Exit;
      }
    }
  }

  if(jprops->iotype == IJL_JBUFF_WRITEHEADER ||
     jprops->iotype == IJL_JFILE_WRITEHEADER)
  {
    jerr = EP_Write_EOI(pState);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed EP_Write_EOI()\n");
      goto Exit;
    }
  }

  /* Flush output buffers. */
  if(NULL != pState->file)
  {
    jerr = Flush_Buffer_To_File(pState);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed Flush_Buffer_To_File()\n");
      goto Exit;
    }
  }

Exit:

  TRACE0(trCALL|trAFTER,"leave Encode_Frame_Init()\n");

  return jerr;
} /* Encode_Frame_Init() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Encode_Frame_Baseline
//
//  Purpose
//    Writes a frame of data.
//    (i.e., it writes some header information if
//    necessary, and then SOF, a single SOS, and
//    EOI segments.
//
//  Context
//
//  Returns
//    Valid error code, or 0 for OK.
//
//  Parameters
//    jprops   Pointer to global IJL properties storage.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Encode_Frame_Baseline(
  JPEG_PROPERTIES* jprops)
{
  int    i;
  SCAN*  scan = NULL;
  STATE* state;
  IJLERR jerr;

#if defined (_LINUX)
  sighandler_t old_fpe_handler;
  sighandler_t old_ill_handler;
  sighandler_t old_segv_handler;

  old_fpe_handler  = signal(SIGFPE,(sighandler_t)own_sig_fpe_ill_segv);
  old_ill_handler  = signal(SIGILL,(sighandler_t)own_sig_fpe_ill_segv);
  old_segv_handler = signal(SIGSEGV,(sighandler_t)own_sig_fpe_ill_segv);
#endif

  TRY_BEGIN

  TRACE0(trCALL|trBEFORE,"enter in Encode_Frame_Baseline()\n");

  jerr  = IJL_OK;
  state = &jprops->state;
  scan  = NULL;

  if(NULL == jprops->jscan)
  {
    scan = ippMalloc(sizeof(SCAN));

    if(NULL == scan)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for SCAN struct\n",sizeof(SCAN));
      jerr = IJL_MEMORY_ERROR;
      goto Exit;
    }

    TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for SCAN struct\n",sizeof(SCAN),scan);

    /* Initialize the scan. */
    scan->ncomps         = jprops->JPGChannels;
    scan->start_spec     = 0;
    scan->end_spec       = 63;
    scan->approx_high    = 0;
    scan->approx_low     = 0;
    scan->restart_interv = 0;
    scan->curxMCU        = 0;
    scan->curyMCU        = 0;
    scan->dc_diff[0]     = 0;
    scan->dc_diff[1]     = 0;
    scan->dc_diff[2]     = 0;
    scan->dc_diff[3]     = 0;

    scan->comps = ippMalloc(sizeof(SCAN_COMPONENT)*scan->ncomps);

    if(NULL == scan->comps)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for SCAN_COMPONENTs\n",
        sizeof(SCAN_COMPONENT) * scan->ncomps);

      jerr = IJL_MEMORY_ERROR;

      goto Exit;
    }

    TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for SCAN_COMPONENT struct\n",
      sizeof(SCAN_COMPONENT) * scan->ncomps,scan->comps);

    for(i = 0; i < scan->ncomps; i++)
    {
      scan->comps[i].comp        = i;
      scan->comps[i].hsampling   = jprops->jframe.comps[i].hsampling;
      scan->comps[i].vsampling   = jprops->jframe.comps[i].vsampling;
      scan->comps[i].dc_table    = NULL;
      scan->comps[i].ac_table    = NULL;
      scan->comps[i].quant_table = NULL;
    }

    jerr = EP_Write_SOF(&jprops->jframe,state);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed EP_Write_SOF()\n");
      goto Exit;
    }

    if(jprops->jframe.restart_interv != 0)
    {
      /* Write a restart header. */
      jerr = EP_Write_DRI(jprops->jframe.restart_interv,state);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed EP_Write_DRI()\n");
        goto Exit;
      }
    }

    /* Need to do this multiple times for jprops->jframe.ncomps > 4. */
    jerr = EP_Write_SOS(
      jprops->HuffIdentifierDC,
      jprops->HuffIdentifierAC,
      scan,
      &jprops->jframe,
      state);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed EP_Write_SOS()\n");
      goto Exit;
    }

    /* Flush temporary buffers to the file/buffer. */
    jerr = Flush_Buffer_To_File(state);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed Flush_Buffer_To_File()\n");
      goto Exit;
    }

    /* Determine which (if any) of the encoding */
    /* fast paths are supported.                */
    jerr = Set_Encode_Fast_Path(jprops);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed Set_Encode_Fast_Path()\n");
      goto Exit;
    }
  }
  else
  {
    scan = jprops->jscan;
  }

  if(jprops->jframe.restart_interv != 0)
  {
    jerr = EncodeScanBaseline(jprops,scan,state);
  }
  else
  {
    jerr = EncodeScanBaseline(jprops,scan,state);
//    jerr = Encode_Scan_Baseline(jprops,scan,state);
  }

  /* If an interrupt was seen, exit after flushing the data buffers. */
  if(jerr == IJL_INTERRUPT_OK || jerr == IJL_ROI_OK)
  {
    jprops->interrupt = FALSE;
    /* Assign the scan structure to a persistent pointer. */
    jprops->jscan = scan;

    TRACE0(trINFO,"interrupt is ok\n");
    goto Exit;
  }

  /* Delete the scan information. */
  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: failed Encode_Scan_Baseline()\n");
    goto Exit;
  }

  /* Delete the scan information. */
  if(IJL_OK == jerr)
  {
    if(NULL != scan)
    {
      if(NULL != scan->comps)
      {
        TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENT's\n",scan->comps);
        ippFree(scan->comps);
        scan->comps = NULL;
      }
      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN\n",scan);
      ippFree(scan);
      scan = NULL;
      jprops->jscan = NULL;
    }

    if(NULL != jprops->jscan)
    {
      if(NULL != jprops->jscan->comps)
      {
        TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENT's\n",jprops->jscan->comps);
        ippFree(jprops->jscan->comps);
        jprops->jscan->comps = NULL;
      }
      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN\n",jprops->jscan);
      ippFree(jprops->jscan);
      jprops->jscan = NULL;
    }
  }

  jerr = EP_Write_EOI(state);

  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: failed EP_Write_EOI()\n");
    goto Exit;
  }

  /* Flush temporary buffers to the file/buffer. */
  jerr = Flush_Buffer_To_File(state);

  TRY_END


  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: exception in Encode_Frame()\n");
  jerr = IJL_EXCEPTION_DETECTED;

  CATCH_END

Exit:

#if defined (_LINUX)
  /* restore signal handlers */
  signal(SIGFPE,(sighandler_t)old_fpe_handler);
  signal(SIGILL,(sighandler_t)old_ill_handler);
  signal(SIGSEGV,(sighandler_t)old_segv_handler);
#endif

  /* do cleanup if any errors */
  if(IJL_OK > jerr)
  {
    if(NULL != scan)
    {
      if(NULL != scan->comps)
      {
        TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENT's\n",scan->comps);
        ippFree(scan->comps);
        scan->comps = NULL;
      }

      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN\n",scan);
      ippFree(scan);
      scan = NULL;
      jprops->jscan = NULL;
    }
  }

  TRACE0(trCALL|trAFTER,"leave Encode_Frame_Baseline()\n");

  return jerr;
} /* Encode_Frame_Baseline() */


LOCFUN(void) Fill_ac_scan(
  JPEG_PROPERTIES* jprops,
  SCAN*            scan,
  int              comp_id,
  int              Ss,
  int              Se,
  int              Ah,
  int              Al)
{
  scan->ncomps             = 1;
  scan->comps[0].comp      = comp_id;
  scan->comps[0].hsampling = jprops->jframe.comps[comp_id].hsampling;
  scan->comps[0].vsampling = jprops->jframe.comps[comp_id].vsampling;
  scan->start_spec         = Ss;
  scan->end_spec           = Se;
  scan->approx_high        = Ah;
  scan->approx_low         = Al;

  return;
} /* fill_ac_scan() */


LOCFUN(void) Fill_dc_scans(
  JPEG_PROPERTIES* jprops,
  SCAN*            scan,
  int              ncomps,
  int              Ah,
  int              Al)
{
  int i;

  if(ncomps <= MAX_COMP_PER_SCAN)
  {
    for(i = 0; i < ncomps; i++)
    {
      scan->comps[i].comp      = i;
      scan->comps[i].hsampling = jprops->jframe.comps[i].hsampling;
      scan->comps[i].vsampling = jprops->jframe.comps[i].vsampling;
    }
    scan->ncomps      = ncomps;
    scan->start_spec  = 0;
    scan->end_spec    = 0;
    scan->approx_high = Ah;
    scan->approx_low  = Al;
  }

  return;
} /* Fill_dc_scans() */


LOCFUN(IJLERR) Detect_number_of_scans(
  JPEG_PROPERTIES* jprops,
  int*             num_scans)
{
  IJLERR jerr = IJL_OK;

  switch(jprops->JPGChannels)
  {
  case 1:
    switch(jprops->JPGColor)
    {
    case IJL_G:
      *num_scans = 6;
      break;

    default:
      *num_scans = 0;
      jerr = IJL_INVALID_JPEG_PROPERTIES;
      break;
    }
    break;

  case 2:
    switch(jprops->JPGColor)
    {
    case IJL_OTHER:
      *num_scans = 5;
      break;

    default:
      *num_scans = 0;
      jerr = IJL_INVALID_JPEG_PROPERTIES;
      break;
    }
    break;

  case 3:
    switch(jprops->JPGColor)
    {
    case IJL_RGB:
      *num_scans = 8;
      break;

    case IJL_YCBCR:
      *num_scans = 10;
      break;

    case IJL_OTHER:
      *num_scans = 8;
      break;

    default:
      *num_scans = 0;
      jerr = IJL_INVALID_JPEG_PROPERTIES;
      break;
    }
    break;

  case 4:
    switch(jprops->JPGColor)
    {
    case IJL_RGBA_FPX:
      *num_scans = 10;
      break;

    case IJL_YCBCRA_FPX:
      *num_scans = 11;
      break;

    case IJL_OTHER:
      *num_scans = 10;
      break;

    default:
      *num_scans = 0;
      jerr = IJL_INVALID_JPEG_PROPERTIES;
      break;
    }
    break;

  default:
    *num_scans = 0;
    jerr = IJL_INVALID_JPEG_PROPERTIES;
    break;
  }

  return jerr;
} /* Detect_number_of_scans() */


LOCFUN(IJLERR) Allocate_scans(
  JPEG_PROPERTIES* jprops,
  int              num_scans)
{
  int    i;
  int    size;
  IJLERR jerr = IJL_OK;

  size = sizeof(SCAN) * num_scans;

  jprops->jscan = ippMalloc(sizeof(SCAN)*num_scans);

  if(NULL == jprops->jscan)
  {
    TRACE1(trERROR,"ERROR: can't allocate %d bytes for SCAN's\n",size);
    jerr = IJL_MEMORY_ERROR;
    goto Exit;
  }

  TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for SCAN's\n",size,jprops->jscan);

  IPPCALL(ippsZero_8u)((Ipp8u*)jprops->jscan,size);

  /* allocate scan components */
  for(i = 0; i < num_scans; i++)
  {
    jprops->jscan[i].comps = ippMalloc(sizeof(SCAN_COMPONENT)*MAX_COMP_PER_SCAN);
    if(NULL == jprops->jscan[i].comps)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for SCAN_COMPONENT's\n",sizeof(SCAN_COMPONENT) * MAX_COMP_PER_SCAN);
      jerr = IJL_MEMORY_ERROR;
      goto Exit;
    }

    TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for SCAN_COMPONENT's\n",sizeof(SCAN_COMPONENT) * MAX_COMP_PER_SCAN,jprops->jscan[i].comps);
  }

Exit:

  return jerr;
} /* Allocate_scans() */


LOCFUN(IJLERR) Make_scan_script(
  JPEG_PROPERTIES* jprops,
  int*             num_scans)
{
  int    i;
  IJLERR jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in Make_scan_script()\n");

  jerr = Detect_number_of_scans(jprops,num_scans);

  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: bad channel-color combination\n");
    goto Exit;
  }

  TRACE1(trINFO,"Number of scan %d\n",*num_scans);

  jerr = Allocate_scans(jprops,*num_scans);

  if(IJL_OK != jerr)
  {
    goto Exit;
  }

  if(jprops->JPGChannels == 1 && jprops->JPGColor == IJL_G)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],1,0,1);
    /* initial AC scan: get some luma data out in hurry */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,2);
    /* Complete spectral selection for luma AC */
    Fill_ac_scan(jprops,&jprops->jscan[2],0,6,63,0,2);
    /* Refine next bit of luma AC */
    Fill_ac_scan(jprops,&jprops->jscan[3],0,1,63,2,1);
    /* Finish DC successive approximation */
    Fill_dc_scans(jprops,&jprops->jscan[4],1,1,0);
    /* Luma bottom bit comes last since it's usually largest scan */
    Fill_ac_scan(jprops,&jprops->jscan[5],0,1,63,1,0);
  }
  else if(jprops->JPGChannels == 2 && jprops->JPGColor == IJL_OTHER)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],2,0,0);
    /* initial AC scan: */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,0);
    /* initial AC scan: */
    Fill_ac_scan(jprops,&jprops->jscan[2],1,1,5,0,0);
    /* Refine AC */
    Fill_ac_scan(jprops,&jprops->jscan[3],0,6,63,0,0);
    /* Refine AC */
    Fill_ac_scan(jprops,&jprops->jscan[4],1,6,63,0,0);
  }
  else if(jprops->JPGChannels == 3 && jprops->JPGColor == IJL_YCBCR)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],3,0,1);
    /* initial AC scan: get some luma data out in hurry */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,2);
    /* chroma data is too small to be worth expending many scans on */
    Fill_ac_scan(jprops,&jprops->jscan[2],2,1,63,0,1);
    Fill_ac_scan(jprops,&jprops->jscan[3],1,1,63,0,1);
    /* Complete spectral selection for luma AC */
    Fill_ac_scan(jprops,&jprops->jscan[4],0,6,63,0,2);
    /* Refine next bit of luma AC */
    Fill_ac_scan(jprops,&jprops->jscan[5],0,1,63,2,1);
    /* Finish DC successive approximation */
    Fill_dc_scans(jprops,&jprops->jscan[6],3,1,0);
    /* Finish AC successive approximation */
    Fill_ac_scan(jprops,&jprops->jscan[7],2,1,63,1,0);
    Fill_ac_scan(jprops,&jprops->jscan[8],1,1,63,1,0);
    /* Luma bottom bit comes last since it's usually largest scan */
    Fill_ac_scan(jprops,&jprops->jscan[9],0,1,63,1,0);
  }
  else if(jprops->JPGChannels == 3 && jprops->JPGColor == IJL_RGB)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],3,0,1);
    /* initial AC scan */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[2],1,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[3],2,1,5,0,0);
    /* Finish DC successive approximation */
    Fill_dc_scans(jprops,&jprops->jscan[4],3,1,0);
    /* Complete spectral selection for AC */
    Fill_ac_scan(jprops,&jprops->jscan[5],0,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[6],1,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[7],2,6,63,0,0);
  }
  else if(jprops->JPGChannels == 3 && jprops->JPGColor == IJL_OTHER)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],3,0,1);
    /* initial AC scan */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[2],1,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[3],2,1,5,0,0);
    /* Finish DC successive approximation */
    Fill_dc_scans(jprops,&jprops->jscan[4],3,1,0);
    /* Complete spectral selection for AC */
    Fill_ac_scan(jprops,&jprops->jscan[5],0,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[6],1,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[7],2,6,63,0,0);
  }
  else if(jprops->JPGChannels == 4 && jprops->JPGColor == IJL_RGBA_FPX)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],4,0,1);
    /* initial AC scan */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[2],1,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[3],2,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[4],3,1,5,0,0);
    /* Finish DC successive approximation */
    Fill_dc_scans(jprops,&jprops->jscan[5],4,1,0);
    /* Complete spectral selection for AC */
    Fill_ac_scan(jprops,&jprops->jscan[6],0,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[7],1,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[8],2,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[9],3,6,63,0,0);
  }
  else if(jprops->JPGChannels == 4 && jprops->JPGColor == IJL_YCBCRA_FPX)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],4,0,1);
    /* initial AC scan: get some luma data out in hurry */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,2);
    /* chroma data is too small to be worth expending many scans on */
    Fill_ac_scan(jprops,&jprops->jscan[2],2,1,63,0,1);
    Fill_ac_scan(jprops,&jprops->jscan[3],1,1,63,0,1);
    /* Alpha data */
    Fill_ac_scan(jprops,&jprops->jscan[4],3,1,63,0,0);
    /* Complete spectral selection for luma AC */
    Fill_ac_scan(jprops,&jprops->jscan[5],0,6,63,0,2);
    /* Refine next bit of luma AC */
    Fill_ac_scan(jprops,&jprops->jscan[6],0,1,63,2,1);
    /* Finish DC successive approximation */
    Fill_dc_scans(jprops,&jprops->jscan[7],4,1,0);
    /* Finish AC successive approximation */
    Fill_ac_scan(jprops,&jprops->jscan[8],2,1,63,1,0);
    Fill_ac_scan(jprops,&jprops->jscan[9],1,1,63,1,0);
    /* Luma bottom bit comes last since it's usually largest scan */
    Fill_ac_scan(jprops,&jprops->jscan[10],0,1,63,1,0);
  }
  else if(jprops->JPGChannels == 4 && jprops->JPGColor == IJL_OTHER)
  {
    /* initial DC scan */
    Fill_dc_scans(jprops,&jprops->jscan[0],4,0,1);
    /* initial AC scan */
    Fill_ac_scan(jprops,&jprops->jscan[1],0,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[2],1,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[3],2,1,5,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[4],3,1,5,0,0);
    /* Finish DC successive approximation */
    Fill_dc_scans(jprops,&jprops->jscan[5],4,1,0);
    /* Complete spectral selection for AC */
    Fill_ac_scan(jprops,&jprops->jscan[6],0,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[7],1,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[8],2,6,63,0,0);
    Fill_ac_scan(jprops,&jprops->jscan[9],3,6,63,0,0);
  }
  else
  {
    jerr = IJL_INVALID_JPEG_PROPERTIES;
  }

Exit:

  if(IJL_OK != jerr)
  {
    /* delete all components */
    for(i = 0; i < *num_scans; i++)
    {
      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENT's\n",jprops->jscan[i].comps);
      ippFree(jprops->jscan[i].comps);
    }
    /* delete all scans */
    TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN's\n",jprops->jscan);
    ippFree(jprops->jscan);
    jprops->jscan = NULL;
  }

  TRACE0(trCALL|trAFTER,"leave Make_scan_script()\n");

  return jerr;
} /* Make_scan_script() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Encode_Frame_Progressive
//
//  Purpose
//    Writes a frame of data.
//    (i.e., it writes some header information if
//    necessary, and then SOF, a single SOS, and
//    EOI segments.
//
//  Context
//
//  Returns
//    Valid error code, or 0 for OK.
//
//  Parameters
//    jprops   Pointer to global IJL properties storage.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Encode_Frame_Progressive(
  JPEG_PROPERTIES* jprops)
{
  int     i, j, k, v, h, jj;
  int     num_scans = 0;
  int     cur_scan;
  int     tbl_ident;
  int     mcu_size;
  int     mcu_row_size;
  int     total_mcu;
  int     blocks_per_mcu;
  int     buf_size;
  int     FREQ[2][256];
  Ipp8u   BITS[16];
  Ipp8u   VALS[256];
  Ipp16s* coef_buf = NULL;
  SCAN*   pScan  = NULL;
  STATE*  pState = &jprops->state;
  IJLERR  jerr  = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in Encode_Frame_Progressive()\n");

#if defined (_LINUX)
  sighandler_t old_fpe_handler;
  sighandler_t old_ill_handler;
  sighandler_t old_segv_handler;

  old_fpe_handler  = signal(SIGFPE,(sighandler_t)own_sig_fpe_ill_segv);
  old_ill_handler  = signal(SIGILL,(sighandler_t)own_sig_fpe_ill_segv);
  old_segv_handler = signal(SIGSEGV,(sighandler_t)own_sig_fpe_ill_segv);
#endif

  TRY_BEGIN

    for(i = 0, blocks_per_mcu = 0; i < jprops->jframe.ncomps; i++)
    {
      blocks_per_mcu += jprops->jframe.comps[i].hsampling *
                        jprops->jframe.comps[i].vsampling;
    }

    total_mcu  = jprops->numxMCUs * jprops->numyMCUs;
    mcu_size   = blocks_per_mcu * DCTSIZE2;

    buf_size = total_mcu * mcu_size;

    jprops->coef_buffer = ippMalloc(sizeof(Ipp16s)*buf_size);

    if(NULL == jprops->coef_buffer)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for coef_buf\n",sizeof(Ipp16s) * buf_size);
      jerr = IJL_MEMORY_ERROR;
      goto Exit;
    }

    TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for coefficient buffer\n",
      buf_size,jprops->coef_buffer);

    if(NULL == jprops->jscan)
    {
      jerr = Make_scan_script(jprops,&num_scans);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: Make_scan_script()\n");
        return jerr;
      }

      if(jprops->iotype != IJL_JBUFF_WRITEWHOLEIMAGE &&
         jprops->iotype != IJL_JFILE_WRITEWHOLEIMAGE)
      {
        /* Abbreviated format for compressed image data. */
        jerr = EP_Write_SOI(pState);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: failed EP_Write_SOI()\n");
          goto Exit;
        }
      }

      jerr = EP_Write_SOF2(&jprops->jframe,pState);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed EP_Write_SOF2()\n");
        goto Exit;
      }

      if(jprops->jframe.restart_interv != 0)
      {
        /* Write a restart header. */
        jerr = EP_Write_DRI(jprops->jframe.restart_interv,pState);

        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: failed EP_Write_DRI()\n");
          goto Exit;
        }
      }

      /* Determine which (if any) of the encoding */
      /* fast paths are supported.                */
      jerr = Set_Encode_Fast_Path(jprops);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed Set_Encode_Fast_Path()\n");
        goto Exit;
      }

      /* do color conversion, DCT and quantization on whole image */
      if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
      {
        jerr = Fill_coeffs_buffer(jprops);
        if(IJL_OK != jerr)
        {
          TRACE0(trERROR,"ERROR: failed Fill_coeffs_buffer()\n");
          goto Exit;
        }
      }
      else
      {
        mcu_row_size = 0;
        for(k = 0; k < jprops->jframe.ncomps; k++)
        {
          mcu_row_size += jprops->jframe.comps[k].vsampling *
                          jprops->jframe.comps[k].hsampling;
        }

        mcu_size = mcu_row_size * DCTSIZE2;
        mcu_row_size *= (jprops->numxMCUs * DCTSIZE2);

        /*                    [][]                                  */
        /* rewrite block from [][] to [][][][] order to coef-buffer */
        for(j = 0; j < jprops->numyMCUs; j++)
        {
          for(i = 0; i < jprops->numxMCUs; i++)
          {
            coef_buf = jprops->coef_buffer + j * mcu_row_size + i * mcu_size;

            for(k = 0; k < jprops->jframe.ncomps; k++)
            {
              /* vertical blocks in MCU */
              for(v = 0; v < jprops->jframe.comps[k].vsampling; v++)
              {
                /* horizontal blocks in MCU */
                for(h = 0; h < jprops->jframe.comps[k].hsampling; h++)
                {
                  unsigned short* cvPtr = jprops->raw_coefs->raw_ptrs[k] +
                  (j * jprops->numxMCUs * jprops->jframe.comps[k].hsampling * jprops->jframe.comps[k].vsampling * DCTSIZE2) +
                  (v * DCTSIZE2 * jprops->jframe.comps[k].hsampling * jprops->numxMCUs) +
                  (i * jprops->jframe.comps[k].hsampling * 8) + h * 8;

                  for(jj = 0; jj < 8; jj++, cvPtr += (jprops->numxMCUs * jprops->jframe.comps[k].hsampling * 8), coef_buf += 8)
                  {
                    IPPCALL(ippsCopy_8u)((const Ipp8u*)cvPtr,(Ipp8u*)coef_buf,sizeof(Ipp16s) << 3);
                  }
                } /* jprops->jframe.comps[k].hsampling */
              } /* jprops->jframe.comps[k].vsampling */
            } /* jprops->jframe.ncomps */
          } /* jprops->numxMCUs */
        } /* jprops->numyMCUs */
      } /* !jprops->raw_coefs || jprops->raw_coefs->data_type */
    } /* if(NULL == jprops->jscan) */

    for(i = 0, cur_scan = 0; i < num_scans; i++)
    {
      pScan = &jprops->jscan[i];

      IPPCALL(ippsZero_8u)((Ipp8u*)FREQ[0],sizeof(FREQ[0]));
      IPPCALL(ippsZero_8u)((Ipp8u*)FREQ[1],sizeof(FREQ[1]));
      IPPCALL(ippsZero_8u)((Ipp8u*)&BITS[0],sizeof(BITS));
      IPPCALL(ippsZero_8u)((Ipp8u*)&VALS[0],sizeof(VALS));

      IPPCALL(ippiEncodeHuffmanStateInit_JPEG_8u)(pState->u.pEncHuffState);

      jerr = Gather_statistics(jprops,pScan,FREQ,pState);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed Gather_statistics()\n");
        goto Exit;
      }

      IPPCALL(ippiEncodeHuffmanStateInit_JPEG_8u)(pState->u.pEncHuffState);

      if(pScan->start_spec == 0 && pScan->end_spec == 0)
      {
        if(pScan->approx_high == 0)
        {
          pScan->dc_diff[0] = 0;
          pScan->dc_diff[1] = 0;
          pScan->dc_diff[2] = 0;
          pScan->dc_diff[3] = 0;

          /* DC scan, luminance component */
          tbl_ident = 0;
          Make_optimal_huff_table(FREQ[tbl_ident],&BITS[0],&VALS[0]);

          jerr = BuildEncoderHuffmanTable(&BITS[0],&VALS[0],&jprops->jFmtDcHuffman[0]);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
            goto Exit;
          }

          jprops->jEncFmtDcHuffman[0] = &jprops->jFmtDcHuffman[0];

          jerr = EP_Write_DHT_Ex(0,0,&BITS[0],&VALS[0],pState);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: failed EP_Write_DHT_Ex()\n");
            goto Exit;
          }

          if(jprops->JPGChannels != 1)
          {
            IPPCALL(ippsZero_8u)(&BITS[0],sizeof(BITS));
            IPPCALL(ippsZero_8u)(&VALS[0],sizeof(VALS));

            /* DC scan, chrominance component */
            tbl_ident = 1;
            Make_optimal_huff_table(FREQ[tbl_ident],&BITS[0],&VALS[0]);

            jerr = BuildEncoderHuffmanTable(&BITS[0],&VALS[0],&jprops->jFmtDcHuffman[1]);
            if(IJL_OK != jerr)
            {
              TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
              goto Exit;
            }

            jprops->jEncFmtDcHuffman[1] = &jprops->jFmtDcHuffman[1];

            jerr = EP_Write_DHT_Ex(0,1,&BITS[0],&VALS[0],pState);

            if(IJL_OK != jerr)
            {
              TRACE0(trERROR,"ERROR: failed EP_Write_DHT_Ex()\n");
              goto Exit;
            }
          }
        }
      }
      else
      {
        if(pScan->comps[0].comp == 0)
        {
          /* AC scan, luminance component */
          tbl_ident = 0;
          jerr = Make_optimal_huff_table(FREQ[tbl_ident],&BITS[0],&VALS[0]);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: Make_optimal_huff_table() failed\n");
            goto Exit;
          }

          jerr = BuildEncoderHuffmanTable(&BITS[0],&VALS[0],&jprops->jFmtAcHuffman[0]);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
            goto Exit;
          }

          jprops->jEncFmtAcHuffman[0] = &jprops->jFmtAcHuffman[0];

          jerr = EP_Write_DHT_Ex(1,0,&BITS[0],&VALS[0],pState);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: failed EP_Write_DHT_Ex()\n");
            goto Exit;
          }
        }
        else
        {
          /* AC scan, chrominance component */
          tbl_ident = 1;
          Make_optimal_huff_table(FREQ[tbl_ident],&BITS[0],&VALS[0]);

          jerr = BuildEncoderHuffmanTable(&BITS[0],&VALS[0],&jprops->jFmtAcHuffman[1]);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: BuildEncoderHuffmanTable() failed\n");
            goto Exit;
          }

          jprops->jEncFmtAcHuffman[1] = &jprops->jFmtAcHuffman[1];

          jerr = EP_Write_DHT_Ex(1,1,&BITS[0],&VALS[0],pState);

          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: failed EP_Write_DHT_Ex()\n");
            goto Exit;
          }
        }
      }

      jerr = EP_Write_SOS(
        jprops->HuffIdentifierDC,
        jprops->HuffIdentifierAC,
        pScan,
        &jprops->jframe,
        pState);

      if(IJL_OK != jerr)
      {
        TRACE0(trERROR,"ERROR: failed EP_Write_SOS()\n");
        goto Exit;
      }

      jerr = EncodeScanProgressive(jprops,pScan,pState);

      if(IJL_OK > jerr)
      {
        TRACE0(trERROR,"ERROR: failed Encode_Scan_Progressive()\n");
        goto Exit;
      }

      cur_scan++;
    } /* for jprops->num_scans */

    jerr = EP_Write_EOI(pState);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed EP_Write_EOI()\n");
      goto Exit;
    }

    /* Flush temporary buffers to the file/buffer. */
    jerr = Flush_Buffer_To_File(pState);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: failed Flush_Buffer_To_File()\n");
      goto Exit;
    }

  TRY_END


  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: exception in Encode_Frame_Progressive()\n");
  jerr = IJL_EXCEPTION_DETECTED;

  CATCH_END

Exit:

#if defined (_LINUX)
  /* restore signal handlers */
  signal(SIGFPE,(sighandler_t)old_fpe_handler);
  signal(SIGILL,(sighandler_t)old_ill_handler);
  signal(SIGSEGV,(sighandler_t)old_segv_handler);
#endif

  /* delete scans and components if need */
  if(NULL != jprops->jscan)
  {
    for(i = 0; i < num_scans; i++)
    {
      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENTs\n",
        jprops->jscan[i].comps);
      ippFree(jprops->jscan[i].comps);
      jprops->jscan[i].comps = NULL;
    }

    TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN's\n",jprops->jscan);
    ippFree(jprops->jscan);
    jprops->jscan = NULL;
  }

  TRACE0(trCALL|trAFTER,"leave Encode_Frame_Progressive()\n");

  return jerr;
} /* Encode_Frame_Progressive() */

