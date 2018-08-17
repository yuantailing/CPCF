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
#ifndef __JPGINTFC_H__
#include "jpgintfc.h"
#endif


#ifdef __INTEL_COMPILER
#pragma warning(disable:171)
#pragma warning(disable:424)
#endif


//EXC_INIT();


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    ijlInit
//
//  Purpose
//    To initalize the IJL.
//
//  Context
//    Always call this before anything else. You should only
//    call this function with a new jcprops structure, or
//    after calling IJL_Free. Otherwise, you may leak dynamically
//    allocated memory.
//
//  Returns
//    Any IJLERR value.
//    IJL_OK indicates success.
//
//  Parameters
//    *jcprops  Pointer to an externally allocated
//              JPEG_CORE_PROPERTIES structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

IJLFUN(IJLERR) ijlInit(
  JPEG_CORE_PROPERTIES* jcprops)
{
  int  i;
  int  bufSize;
  int  encBufSize;
  int  decBufSize;
  IJLERR jerr = IJL_OK;
  IJL_CONTEXT* ctx = NULL;
  JPEG_PROPERTIES* jprops;

#if defined (_LINUX)
  sighandler_t old_fpe_handler;
  sighandler_t old_ill_handler;
  sighandler_t old_segv_handler;

  old_fpe_handler  = signal(SIGFPE,(sighandler_t)own_sig_fpe_ill_segv);
  old_ill_handler  = signal(SIGILL,(sighandler_t)own_sig_fpe_ill_segv);
  old_segv_handler = signal(SIGSEGV,(sighandler_t)own_sig_fpe_ill_segv);
#endif

  TRY_BEGIN

    jprops = &jcprops->jprops;

    ctx = ippMalloc(sizeof(IJL_CONTEXT));
    if(NULL == ctx)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for IJL_CONTEXT\n",
        sizeof(IJL_CONTEXT));
      jerr = IJL_MEMORY_ERROR;
      goto Exit;
    }

    IPPCALL(ippiEncodeHuffmanStateGetBufSize_JPEG_8u)(&encBufSize);
    IPPCALL(ippiDecodeHuffmanStateGetBufSize_JPEG_8u)(&decBufSize);

    bufSize = IPP_MAX(encBufSize,decBufSize);

    ctx->pHuffStateBuf = ippMalloc(bufSize);
    if(NULL == ctx->pHuffStateBuf)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for Huffman state\n",
        bufSize);
      jerr = IJL_MEMORY_ERROR;
      goto Exit;
    }

    jprops->state.ctx = ctx;

    ownInitializeLibrary(ctx);

    TRACE0(trCALL|trBEFORE,"enter in ijlInit\n");

    /* Advanced control options. */
    jcprops->UseJPEGPROPERTIES            = 0;

    /* I/O data specifiers - DIB Specific. */
    jcprops->DIBBytes                     = NULL;
    jcprops->DIBWidth                     = 0;
    jcprops->DIBHeight                    = 0;
    jcprops->DIBPadBytes                  = 0;
    jcprops->DIBChannels                  = 3;
    jcprops->DIBColor                     = IJL_BGR;
    jcprops->DIBSubsampling               = IJL_NONE;

    /* I/O data specifiers - JPEG Specific. */
    jcprops->JPGBytes                     = NULL;
    jcprops->JPGFile                      = NULL;
    jcprops->JPGSizeBytes                 = 0;
    jcprops->JPGWidth                     = 0;
    jcprops->JPGHeight                    = 0;
    jcprops->JPGChannels                  = 3;
    jcprops->JPGColor                     = IJL_YCBCR;
    jcprops->JPGSubsampling               = IJL_411;
    jcprops->JPGThumbWidth                = 0;
    jcprops->JPGThumbHeight               = 0;

    /* JPEG conversion properties. */
    jcprops->cconversion_reqd             = TRUE;
    jcprops->upsampling_reqd              = TRUE;
    jcprops->jquality                     = 75;

    jprops->jinterleaveType               = 0;
    jprops->fast_processing               = IJL_NO_CC_OR_US;
    jprops->interrupt                     = FALSE;

    /* Compression/Decompression control. */
    jprops->iotype                        = IJL_SETUP;

    jprops->roi.left                      = 0;
    jprops->roi.top                       = 0;
    jprops->roi.right                     = 0;
    jprops->roi.bottom                    = 0;

    jprops->dcttype                       = IJL_IPP;

    jprops->DIBBytes                      = jcprops->DIBBytes;
    jprops->DIBWidth                      = jcprops->DIBWidth;
    jprops->DIBHeight                     = jcprops->DIBHeight;
    jprops->DIBPadBytes                   = jcprops->DIBPadBytes;
    jprops->DIBChannels                   = jcprops->DIBChannels;
    jprops->DIBColor                      = jcprops->DIBColor;
    jprops->DIBSubsampling                = jcprops->DIBSubsampling;
    jprops->DIBLineBytes                  = 0;

    jprops->JPGBytes                      = jcprops->JPGBytes;
    jprops->JPGFile                       = jcprops->JPGFile;
    jprops->JPGSizeBytes                  = jcprops->JPGSizeBytes;
    jprops->JPGWidth                      = jcprops->JPGWidth;
    jprops->JPGHeight                     = jcprops->JPGHeight;
    jprops->JPGChannels                   = jcprops->JPGChannels;
    jprops->JPGColor                      = jcprops->JPGColor;
    jprops->JPGSubsampling                = jcprops->JPGSubsampling;
    jprops->JPGThumbWidth                 = jcprops->JPGThumbWidth;
    jprops->JPGThumbHeight                = jcprops->JPGThumbHeight;

    jprops->cconversion_reqd              = jcprops->cconversion_reqd;
    jprops->upsampling_reqd               = jcprops->upsampling_reqd;
    jprops->jquality                      = jcprops->jquality;

    /* Sampling. */
    jprops->numxMCUs                      = 0;
    jprops->numyMCUs                      = 0;

    /* Tables. */
    jprops->use_external_qtables          = 0;
    jprops->use_external_htables          = 0;

    jprops->jEncFmtQuant[0]               = NULL;
    jprops->jEncFmtQuant[1]               = NULL;
    jprops->jEncFmtQuant[2]               = NULL;
    jprops->jEncFmtQuant[3]               = NULL;

    jprops->jEncFmtAcHuffman[0]           = NULL;
    jprops->jEncFmtAcHuffman[1]           = NULL;
    jprops->jEncFmtAcHuffman[2]           = NULL;
    jprops->jEncFmtAcHuffman[3]           = NULL;

    jprops->jEncFmtDcHuffman[0]           = NULL;
    jprops->jEncFmtDcHuffman[1]           = NULL;
    jprops->jEncFmtDcHuffman[2]           = NULL;
    jprops->jEncFmtDcHuffman[3]           = NULL;

    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtQuant[0],sizeof(QUANT_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtQuant[1],sizeof(QUANT_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtQuant[2],sizeof(QUANT_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtQuant[3],sizeof(QUANT_TABLE));

    {
      Ipp16s* ptr0 = &jprops->jFmtQuant[0].elarray[0];
      Ipp16s* ptr1 = &jprops->jFmtQuant[1].elarray[0];
      Ipp16s* ptr2 = &jprops->jFmtQuant[2].elarray[0];
      Ipp16s* ptr3 = &jprops->jFmtQuant[3].elarray[0];

      jprops->jFmtQuant[0].elements = (Ipp16u*)OWN_ALIGN_PTR(ptr0,8);
      jprops->jFmtQuant[1].elements = (Ipp16u*)OWN_ALIGN_PTR(ptr1,8);
      jprops->jFmtQuant[2].elements = (Ipp16u*)OWN_ALIGN_PTR(ptr2,8);
      jprops->jFmtQuant[3].elements = (Ipp16u*)OWN_ALIGN_PTR(ptr3,8);
    }

    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtDcHuffman[0],sizeof(HUFFMAN_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtDcHuffman[1],sizeof(HUFFMAN_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtDcHuffman[2],sizeof(HUFFMAN_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtDcHuffman[3],sizeof(HUFFMAN_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtAcHuffman[0],sizeof(HUFFMAN_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtAcHuffman[1],sizeof(HUFFMAN_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtAcHuffman[2],sizeof(HUFFMAN_TABLE));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->jFmtAcHuffman[3],sizeof(HUFFMAN_TABLE));

    IPPCALL(ippiEncodeHuffmanSpecGetBufSize_JPEG_8u)(&encBufSize);
    IPPCALL(ippiDecodeHuffmanSpecGetBufSize_JPEG_8u)(&decBufSize);

    bufSize = IPP_MAX(encBufSize,decBufSize);

    for(i = 0; i < 4; i++)
    {
      jprops->jFmtDcHuffman[i].u.pDecHuffTbl =
        (IppiDecodeHuffmanSpec*)ippMalloc(bufSize);
      if(NULL == jprops->jFmtDcHuffman[i].u.pDecHuffTbl)
      {
        TRACE1(trERROR,"ERROR: can't allocate %d bytes for Huffman table\n",
          bufSize);
        jerr = IJL_MEMORY_ERROR;
        goto Exit;
      }

      jprops->jFmtAcHuffman[i].u.pDecHuffTbl =
        (IppiDecodeHuffmanSpec*)ippMalloc(bufSize);
      if(NULL == jprops->jFmtAcHuffman[i].u.pDecHuffTbl)
      {
        TRACE1(trERROR,"ERROR: can't allocate %d bytes for Huffman table\n",
          bufSize);
        jerr = IJL_MEMORY_ERROR;
        goto Exit;
      }
    }

    jprops->needframe                     = TRUE;

    /* Frame-specific members. */
    jprops->jframe.precision              = 0;
    jprops->jframe.width                  = 0;
    jprops->jframe.height                 = 0;
    jprops->jframe.MCUheight              = 0;
    jprops->jframe.MCUwidth               = 0;
    jprops->jframe.max_hsampling          = 0;
    jprops->jframe.max_vsampling          = 0;
    jprops->jframe.ncomps                 = MAX_COMP_PER_SCAN;
    jprops->jframe.horMCU                 = 0;
    jprops->jframe.totalMCU               = 0;

    jprops->jframe.comps = ippMalloc(sizeof(FRAME_COMPONENT)*jprops->jframe.ncomps);

    if(NULL == jprops->jframe.comps)
    {
      TRACE1(trERROR,"ERROR: can't allocate %d bytes for FRAME_COMPONENTs\n",
        sizeof(FRAME_COMPONENT)*jprops->jframe.ncomps);
      jerr = IJL_MEMORY_ERROR;
      goto Exit;
    }

    TRACE2(trMEMORY,"allocate %d bytes at 0x%08X for FRAME_COMPONENTs\n",
      sizeof(FRAME_COMPONENT) * jprops->jframe.ncomps,jprops->jframe.comps);

    IPPCALL(ippsZero_8u)(
      (Ipp8u*)jprops->jframe.comps,
      sizeof(FRAME_COMPONENT)*jprops->jframe.ncomps);

    jprops->jframe.restart_interv         = 0;
    jprops->jframe.SeenAllDCScans         = 0;
    jprops->jframe.SeenAllACScans         = 0;

    jprops->SawAdobeMarker                = FALSE;
    jprops->AdobeXform                    = 0;

    /* persist scan pointer */
    jprops->jscan                         = NULL;

    /* The row-offsets structure is not allocated by default. */
    jprops->rowoffsets                    = NULL;

    /* Raw tables will be filled in ijlWrite function, if they are default,  */
    /* or in ijlRead when DHT or DQT marker is parsed or custom by user.     */
    /* In two last cases user must allocate memory and set cleared pointers. */

    jprops->rawquanttables[0].quantizer   = NULL;
    jprops->rawquanttables[0].ident       = 0;

    jprops->rawquanttables[1].quantizer   = NULL;
    jprops->rawquanttables[1].ident       = 0;

    jprops->rawquanttables[2].quantizer   = NULL;
    jprops->rawquanttables[2].ident       = 0;

    jprops->rawquanttables[3].quantizer   = NULL;
    jprops->rawquanttables[3].ident       = 0;

    jprops->rawhufftables[0].bits         = NULL;
    jprops->rawhufftables[0].vals         = NULL;
    jprops->rawhufftables[0].hclass       = 0;
    jprops->rawhufftables[0].ident        = 0;

    jprops->rawhufftables[1].bits         = NULL;
    jprops->rawhufftables[1].vals         = NULL;
    jprops->rawhufftables[1].hclass       = 0;
    jprops->rawhufftables[1].ident        = 0;

    jprops->rawhufftables[2].bits         = NULL;
    jprops->rawhufftables[2].vals         = NULL;
    jprops->rawhufftables[2].hclass       = 0;
    jprops->rawhufftables[2].ident        = 0;

    jprops->rawhufftables[3].bits         = NULL;
    jprops->rawhufftables[3].vals         = NULL;
    jprops->rawhufftables[3].hclass       = 0;
    jprops->rawhufftables[3].ident        = 0;

    jprops->rawhufftables[4].bits         = NULL;
    jprops->rawhufftables[4].vals         = NULL;
    jprops->rawhufftables[4].hclass       = 0;
    jprops->rawhufftables[4].ident        = 0;

    jprops->rawhufftables[5].bits         = NULL;
    jprops->rawhufftables[5].vals         = NULL;
    jprops->rawhufftables[5].hclass       = 0;
    jprops->rawhufftables[5].ident        = 0;

    jprops->rawhufftables[6].bits         = NULL;
    jprops->rawhufftables[6].vals         = NULL;
    jprops->rawhufftables[6].hclass       = 0;
    jprops->rawhufftables[6].ident        = 0;

    jprops->rawhufftables[7].bits         = NULL;
    jprops->rawhufftables[7].vals         = NULL;
    jprops->rawhufftables[7].hclass       = 0;
    jprops->rawhufftables[7].ident        = 0;

    /* Next standard values are useful for ijlRead and ijlWrite. */
    jprops->nhuffActables = 0;
    jprops->nhuffDctables = 0;
    jprops->maxhuffindex  = 0;
    jprops->maxquantindex = 0;
    jprops->nqtables      = 0;
    jprops->HuffIdentifierAC[0] = 0;
    jprops->HuffIdentifierDC[0] = 0;
    jprops->HuffIdentifierAC[1] = 1;
    jprops->HuffIdentifierDC[1] = 1;
    jprops->HuffIdentifierAC[2] = 1;
    jprops->HuffIdentifierDC[2] = 1;
    jprops->HuffIdentifierAC[3] = 1;
    jprops->HuffIdentifierDC[3] = 1;
    jprops->jframe.comps[0].quant_sel = 0;
    jprops->jframe.comps[1].quant_sel = 1;
    jprops->jframe.comps[2].quant_sel = 1;
    jprops->jframe.comps[3].quant_sel = 1;

    /* State specific members. */
    jprops->state.file                    = NULL;
    jprops->state.cur_entropy_ptr         = NULL;
    jprops->state.start_entropy_ptr       = NULL;
    jprops->state.end_entropy_ptr         = NULL;
    jprops->state.entropy_bytes_processed = 0;
    jprops->state.DIB_ptr                 = NULL;
    jprops->state.entropy_bytes_left      = 0;

    jprops->state.processor_type          = ippGetCpuType();

    jprops->state.entropy_buf_maxsize     = 0;
    jprops->state.unread_marker           = 0;
    jprops->state.cur_scan_comp           = 0;

    /* Intermediate buffers. */
    jprops->MCUBuf                        = NULL;

    /* CPU-ID specific members. */
    jprops->processor_type                = jprops->state.processor_type;

    /* pointer to buffer for raw DCT coefficients */
    jprops->raw_coefs                     = NULL;

    jprops->progressive_found             = 0;

    /* pointer to buffer for progressive mode */
    jprops->coef_buffer                   = NULL;

    /* new IJL 1.5 fields */
    jprops->upsampling_type               = IJL_BOX_FILTER;
    jprops->sampling_state_ptr            = NULL;

    jprops->AdobeVersion                  = 100;
    jprops->AdobeFlags0                   = 0;
    jprops->AdobeFlags1                   = 0;

    jprops->jfif_app0_detected            = 0;
    jprops->jfif_app0_version             = 0x0101;
    jprops->jfif_app0_units               = 0; /* pixel */
    jprops->jfif_app0_Xdensity            = 1;
    jprops->jfif_app0_Ydensity            = 1;

    jprops->jpeg_comment                  = NULL;
    jprops->jpeg_comment_size             = 0;

  TRY_END

  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: EXCEPTION detected in ijlInit\n");
  jerr = IJL_EXCEPTION_DETECTED;

  CATCH_END

Exit:

#if defined (_LINUX)
  /* restore signal handlers */
  signal(SIGFPE,(sighandler_t)old_fpe_handler);
  signal(SIGILL,(sighandler_t)old_ill_handler);
  signal(SIGSEGV,(sighandler_t)old_segv_handler);
#endif

  if(IJL_OK > jerr)
  {
    if(NULL != jcprops->jprops.jframe.comps)
    {
      TRACE1(trMEMORY,"freeing memory at 0x%08X for FRAME_COMPONENTs\n",
        jcprops->jprops.jframe.comps);
      ippFree(jcprops->jprops.jframe.comps);
      jcprops->jprops.jframe.comps  = NULL;
      jcprops->jprops.jframe.ncomps = 0;
    }

    if(NULL != jcprops->jprops.coef_buffer)
    {
      TRACE1(trMEMORY,"freeing memory at 0x%08X for coefficient buffer\n",
        jcprops->jprops.coef_buffer);
      ippFree(jcprops->jprops.coef_buffer);
      jcprops->jprops.coef_buffer = NULL;
    }

    if(NULL != jcprops->jprops.rowoffsets)
    {
      TRACE1(trMEMORY,"freeing memory at 0x%08X for ENTROPYSTRUCTs\n",
        jcprops->jprops.rowoffsets);
      ippFree(jcprops->jprops.rowoffsets);
      jcprops->jprops.rowoffsets = NULL;
    }

    if(NULL != jcprops->jprops.jscan)
    {
      if(NULL != jcprops->jprops.jscan->comps)
      {
        TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENTs\n",
          jcprops->jprops.jscan->comps);
        ippFree(jcprops->jprops.jscan->comps);
        jcprops->jprops.jscan->comps = NULL;
      }

      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN struct\n",
        jcprops->jprops.jscan);
      ippFree(jcprops->jprops.jscan);
      jcprops->jprops.jscan = NULL;
    }

    for(i = 0; i < 4; i++)
    {
      if(NULL != jcprops->jprops.jFmtDcHuffman[i].u.pDecHuffTbl)
      {
        ippFree(jcprops->jprops.jFmtDcHuffman[i].u.pDecHuffTbl);
        jcprops->jprops.jFmtDcHuffman[i].u.pDecHuffTbl = NULL;
      }

      if(NULL != jcprops->jprops.jFmtAcHuffman[i].u.pDecHuffTbl)
      {
        ippFree(jcprops->jprops.jFmtAcHuffman[i].u.pDecHuffTbl);
        jcprops->jprops.jFmtAcHuffman[i].u.pDecHuffTbl = NULL;
      }
    }
  }

  TRACE0(trCALL|trAFTER,"leave ijlInit\n");

  return jerr;
} /* ijlInit() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    ijlFree
//
//  Purpose
//    To close the IJL.
//
//  Context
//    Always call this when done using the IJL.  It performs
//    cleanup of dynamically allocated memory.  You will have
//    to call IJL_Init to use the IJL again.
//
//  Returns
//    Any IJLERR value.
//    IJL_OK indicates success.
//
//  Parameters
//    *jcprops  Pointer to an externally allocated
//              JPEG_CORE_PROPERTIES structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

IJLFUN(IJLERR) ijlFree(
  JPEG_CORE_PROPERTIES* jcprops)
{
  int    i;
  IJLERR jerr = IJL_OK;
  IJL_CONTEXT* ctx;
  JPEG_PROPERTIES *jprops;

#if defined (_LINUX)
  sighandler_t old_fpe_handler;
  sighandler_t old_ill_handler;
  sighandler_t old_segv_handler;

  old_fpe_handler  = signal(SIGFPE,(sighandler_t)own_sig_fpe_ill_segv);
  old_ill_handler  = signal(SIGILL,(sighandler_t)own_sig_fpe_ill_segv);
  old_segv_handler = signal(SIGSEGV,(sighandler_t)own_sig_fpe_ill_segv);
#endif

  TRY_BEGIN

  TRACE0(trCALL|trBEFORE,"enter in ijlFree\n");

    jprops = &(jcprops->jprops);

    /* If a file is still open, close it. */
    if(jprops->state.file)
    {
      if(FALSE == ownCloseFile(jprops->state.file))
      {
        TRACE1(trERROR,"ERROR: Can't close file %s\n",jcprops->JPGFile);
        jerr = IJL_ERR_FILECLOSE;
        goto Exit;
      }
      jprops->state.file = NULL;
    }

    /* encoder formatted tables */
    for(i = 0; i < 4; i++)
    {
      jprops->jEncFmtQuant[i]     = NULL;
      jprops->jEncFmtDcHuffman[i] = NULL;
      jprops->jEncFmtAcHuffman[i] = NULL;
    }

    if(jprops->jframe.ncomps > 255)
    {
      TRACE1(trERROR,"ERROR: jprops->jframe.ncomps is %d\n",jprops->jframe.ncomps);
      jerr = IJL_INVALID_JPEG_PROPERTIES;
      goto Exit;
    }

    /* Free up the FRAME_COMPONENT structures. */
    if(NULL != jprops->jframe.comps)
    {
      TRACE1(trMEMORY,"freeing memory at 0x%08X for FRAME_COMPONENTs\n",jprops->jframe.comps);
      ippFree(jprops->jframe.comps);
      jprops->jframe.comps  = NULL;
      jprops->jframe.ncomps = 0;
    }

    if(NULL != jprops->coef_buffer)
    {
      TRACE1(trMEMORY,"freeing memory at 0x%08X for coefficient buffer\n",jprops->coef_buffer);
      ippFree(jprops->coef_buffer);
      jprops->coef_buffer = NULL;
    }

    if((IJL_TRIANGLE_FILTER == jprops->upsampling_type) &&
       (NULL != jprops->sampling_state_ptr))
    {
      if(jprops->sampling_state_ptr->top_row > jprops->sampling_state_ptr->cur_row)
        jprops->sampling_state_ptr->top_row = jprops->sampling_state_ptr->cur_row;

      if(jprops->sampling_state_ptr->top_row > jprops->sampling_state_ptr->bottom_row)
        jprops->sampling_state_ptr->top_row = jprops->sampling_state_ptr->bottom_row;

      if(jprops->sampling_state_ptr->top_row > jprops->sampling_state_ptr->last_row)
        jprops->sampling_state_ptr->top_row = jprops->sampling_state_ptr->last_row;


      TRACE1(trMEMORY,"freeing memory at 0x%08X for upsampling row buffer\n",jprops->sampling_state_ptr->top_row);
      ippFree(jprops->sampling_state_ptr->top_row);

      TRACE1(trMEMORY,"freeing memory at 0x%08X for upsampling state\n",jprops->sampling_state_ptr);
      ippFree(jprops->sampling_state_ptr);
      jprops->sampling_state_ptr = NULL;
    }

    /* free roi offset buffer */
    if(NULL != jprops->rowoffsets)
    {
      for(i = 0; i < MAX_MCU_ROWS; i++)
      {
        if(NULL != jprops->rowoffsets[i].pDecHuffState)
        {
          IPPCALL(ippiDecodeHuffmanStateFree_JPEG_8u)(jprops->rowoffsets[i].pDecHuffState);
        }

        jprops->rowoffsets[i].offset = 0xFFFFFFFF; /* uninitialized value */
      }
      TRACE1(trMEMORY,"freeing memory at 0x%08X for ENTROPYSTRUCT's\n",jprops->rowoffsets);
      ippFree(jprops->rowoffsets);
      jprops->rowoffsets = NULL;
    }

    /* free persist scan */
    if(NULL != jprops->jscan)
    {
      if(NULL != jprops->jscan->comps)
      {
        TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN_COMPONENT's\n",jprops->jscan->comps);
        ippFree(jprops->jscan->comps);
        jprops->jscan->comps = NULL;
      }
      TRACE1(trMEMORY,"freeing memory at 0x%08X for SCAN struct\n",jprops->jscan);
      ippFree(jprops->jscan);
      jprops->jscan = NULL;
    }

    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->rawquanttables,sizeof(jprops->rawquanttables));
    IPPCALL(ippsZero_8u)((Ipp8u*)&jprops->rawhufftables,sizeof(jprops->rawhufftables));

    for(i = 0; i < 4; i++)
    {
      if(NULL != jprops->jFmtDcHuffman[i].u.pDecHuffTbl)
      {
        ippFree(jprops->jFmtDcHuffman[i].u.pDecHuffTbl);
        jprops->jFmtDcHuffman[i].u.pDecHuffTbl = NULL;
      }

      if(NULL != jprops->jFmtAcHuffman[i].u.pDecHuffTbl)
      {
        ippFree(jprops->jFmtAcHuffman[i].u.pDecHuffTbl);
        jprops->jFmtAcHuffman[i].u.pDecHuffTbl = NULL;
      }
    }

    jprops->nqtables             = 0;
    jprops->maxquantindex        = 0;
    jprops->maxhuffindex         = 0;
    jprops->nhuffActables        = 0;
    jprops->nhuffDctables        = 0;
    jprops->use_external_qtables = 0;
    jprops->use_external_htables = 0;

  TRY_END

  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: EXCEPTION detected in ijlFree\n");
  jerr = IJL_EXCEPTION_DETECTED;

  CATCH_END

Exit:

#if defined (_LINUX)
  /* restore signal handlers */
  signal(SIGFPE,(sighandler_t)old_fpe_handler);
  signal(SIGILL,(sighandler_t)old_ill_handler);
  signal(SIGSEGV,(sighandler_t)old_segv_handler);
#endif

  TRACE0(trCALL|trAFTER,"leave ijlFree\n");

  ctx = (IJL_CONTEXT*)jcprops->jprops.state.ctx;

  if(NULL != ctx)
  {
    ownFreeLibrary(ctx);

    if(NULL != ctx->pHuffStateBuf)
    {
      ippFree(ctx->pHuffStateBuf);
      ctx->pHuffStateBuf = NULL;
    }
    ippFree(ctx);
    jcprops->jprops.state.ctx = NULL;
  }

  return jerr;
} /* ijlFree() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    CheckParameters
//
//  Purpose
//    Validate that the properties specified in the jcprops
//    struct are reasonable.
//
//  Context
//    Called on both encode and decode.
//
//  Returns
//    Any IJLERR value.
//    IJL_OK indicates success.
//
//  Parameters:
//    *jcprops  Pointer to an externally allocated
//              JPEG_CORE_PROPERTIES structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

LOCFUN(IJLERR) CheckParameters(
  JPEG_CORE_PROPERTIES* jcprops)
{
  BOOL read_test         = FALSE;
  BOOL write_test        = FALSE;
  BOOL file_test         = FALSE;
  BOOL need_DIB_test     = FALSE;
  BOOL color_combination = FALSE;
  JPEG_PROPERTIES* jprops;

  TRACE0(trCALL|trBEFORE,"enter in CheckParameters()\n");

  jprops = &(jcprops->jprops);

  /* If UseJPEGProperties is asserted, then we assume that */
  /* the user is performing their own parameter checking.  */
  if(jcprops->UseJPEGPROPERTIES != 0)
  {
    TRACE0(trINFO,"UseJPEGProperties is asserted\n");
    TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
    return IJL_OK;
  }

  /* Read/write test. */
  if(jprops->iotype == IJL_SETUP)
  {
    TRACE0(trERROR,"ERROR: jprops->iotype == IJL_SETUP\n");
    TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
    return IJL_INVALID_JPEG_PROPERTIES;
  }
  else if((jprops->iotype == IJL_JFILE_READPARAMS) ||
          (jprops->iotype == IJL_JBUFF_READPARAMS) ||
          (jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
          (jprops->iotype == IJL_JBUFF_READWHOLEIMAGE) ||
          (jprops->iotype == IJL_JFILE_READHEADER) ||
          (jprops->iotype == IJL_JBUFF_READHEADER) ||
          (jprops->iotype == IJL_JFILE_READENTROPY) ||
          (jprops->iotype == IJL_JBUFF_READENTROPY) ||
          (jprops->iotype == IJL_JFILE_READONEHALF) ||
          (jprops->iotype == IJL_JBUFF_READONEHALF) ||
          (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
          (jprops->iotype == IJL_JBUFF_READONEQUARTER) ||
          (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
          (jprops->iotype == IJL_JBUFF_READONEEIGHTH) ||
          (jprops->iotype == IJL_JFILE_READTHUMBNAIL) ||
          (jprops->iotype == IJL_JBUFF_READTHUMBNAIL))
  {
    read_test = TRUE;
  }
  else if((jprops->iotype == IJL_JFILE_WRITEWHOLEIMAGE) ||
          (jprops->iotype == IJL_JBUFF_WRITEWHOLEIMAGE) ||
          (jprops->iotype == IJL_JFILE_WRITEHEADER) ||
          (jprops->iotype == IJL_JBUFF_WRITEHEADER) ||
          (jprops->iotype == IJL_JFILE_WRITEENTROPY) ||
          (jprops->iotype == IJL_JBUFF_WRITEENTROPY))
  {
    write_test = TRUE;
  }
  else
  {
    TRACE0(trERROR,"ERROR: jprops->iotype must be read or write operation\n");
    TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
    return IJL_INVALID_JPEG_PROPERTIES;
  }

  /* file/buffer test */
  if((jprops->iotype == IJL_JFILE_READPARAMS) ||
     (jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_READHEADER) ||
     (jprops->iotype == IJL_JFILE_READENTROPY) ||
     (jprops->iotype == IJL_JFILE_READONEHALF) ||
     (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
     (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
     (jprops->iotype == IJL_JFILE_READTHUMBNAIL) ||
     (jprops->iotype == IJL_JFILE_WRITEWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_WRITEHEADER) ||
     (jprops->iotype == IJL_JFILE_WRITEENTROPY))
  {
    file_test = TRUE;
  }

  /* Does iotype imply a need for a DIB? */
  if((jprops->iotype == IJL_JFILE_READWHOLEIMAGE)||
     (jprops->iotype == IJL_JBUFF_READWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_READENTROPY) ||
     (jprops->iotype == IJL_JBUFF_READENTROPY) ||
     (jprops->iotype == IJL_JFILE_WRITEWHOLEIMAGE) ||
     (jprops->iotype == IJL_JBUFF_WRITEWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_WRITEENTROPY) ||
     (jprops->iotype == IJL_JBUFF_WRITEENTROPY) ||
     (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
     (jprops->iotype == IJL_JBUFF_READONEEIGHTH) ||
     (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
     (jprops->iotype == IJL_JBUFF_READONEQUARTER) ||
     (jprops->iotype == IJL_JBUFF_READONEHALF) ||
     (jprops->iotype == IJL_JBUFF_READONEHALF) ||
     (jprops->iotype == IJL_JFILE_READTHUMBNAIL) ||
     (jprops->iotype == IJL_JBUFF_READTHUMBNAIL))
  {
      need_DIB_test = TRUE;
  }

  /* Check for valid DIB properties. */
  if((need_DIB_test == TRUE) && (jprops->raw_coefs == NULL)) // OA: don't check if raw_coefs
  {
    /* The DIB pointer needs to be valid. */
    if(jcprops->DIBBytes == NULL)
    {
      TRACE0(trERROR,"ERROR: jcprops->DIBBytes == NULL\n");
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_INVALID_JPEG_PROPERTIES;
    }

    /* Bound the DIB width. */
    if((jcprops->DIBWidth > 65535) || (jcprops->DIBWidth < 1))
    {
      TRACE1(trERROR,"ERROR: jcprops->DIBWidth is %d\n",jcprops->DIBWidth);
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_INVALID_JPEG_PROPERTIES;
    }

    /* Bound the DIB height. */
    if((jcprops->DIBHeight > 65535) || (jcprops->DIBHeight < -65535))
    {
      TRACE1(trERROR,"ERROR: jcprops->DIBHeight is %d\n",jcprops->DIBHeight);
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_INVALID_JPEG_PROPERTIES;
    }

    /* Check the thumbnail output DIB.       */
    /* It has to be at least 256x256 pixels. */
    if((jprops->iotype == IJL_JFILE_READTHUMBNAIL) ||
       (jprops->iotype == IJL_JBUFF_READTHUMBNAIL))
    {
      if((jcprops->DIBColor != IJL_RGB) && (jcprops->DIBColor != IJL_BGR))
      {
        TRACE0(trERROR,"ERROR: unexpected thumbnails color model\n");
        TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
        return IJL_THUMBNAIL_DIB_WRONG_COLOR;
      }
    }

    if(jprops->raw_coefs == NULL)
    {
      /* Check DIBpadbytes. */
      if(jcprops->DIBPadBytes < 0)
      {
        TRACE1(trERROR,"ERROR: jcprops->DIBPadBytes is %d\n",
          jcprops->DIBPadBytes);
        TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
        return IJL_INVALID_JPEG_PROPERTIES;
      }

      /* Bound the number of DIB Channels. */
      if((jcprops->DIBChannels < 1) || (jcprops->DIBChannels > 255))
      {
        TRACE1(trERROR,"ERROR: jcprops->DIBChannels is %d\n",
          jcprops->DIBChannels);
        TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
        return IJL_INVALID_JPEG_PROPERTIES;
      }

      /* Check for a valid DIB subsampling. */
      if(jcprops->DIBSubsampling != IJL_NONE)
      {
        if ((jcprops->DIBSubsampling != IJL_422) || (jcprops->DIBColor != IJL_YCBCR))
        {
          TRACE0(trERROR,"ERROR: unexpected jcprops->DIBSubsampling\n");
          TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
          return IJL_UNSUPPORTED_SUBSAMPLING;
        }
      }
    }
  }

  /* Check for valid JPEG properties. */
  if(FALSE != file_test)
  {
    /* If we are to read/write from a file, then the file name */
    /* needs to be supplied.                                   */
    if(NULL == jcprops->JPGFile)
    {
      TRACE0(trERROR,"ERROR: NULL == jcprops->JPGFile\n");
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_INVALID_JPEG_PROPERTIES;
    }
  }
  else
  {
    /* If we are to read/write from a buffer, then */
    /* the buffer pointer needs to be valid.       */
    if(NULL == jcprops->JPGBytes)
    {
      TRACE0(trERROR,"ERROR: NULL == jcprops->JPGBytes\n");
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_INVALID_JPEG_PROPERTIES;
    }

    if(jcprops->JPGSizeBytes == 0)
    {
      TRACE1(trERROR,"ERROR: jcprops->JPGSizeBytes - %d\n",
        jcprops->JPGSizeBytes);
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_INVALID_JPEG_PROPERTIES;
    }
  }

  /* Bound the number of JPEG Channels. */
  if((jcprops->JPGChannels < 1) || (jcprops->JPGChannels > 256))
  {
    TRACE1(trERROR,"ERROR: jcprops->JPGChannels is %d\n",jcprops->JPGChannels);
    TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
    return IJL_INVALID_JPEG_PROPERTIES;
  }

  /* Are we encoding where we are writing image data? */
  if(FALSE != write_test)
  {
    if(FALSE != need_DIB_test)
    {
      /* If writing a JPEG image, then the width and */
      /* height must be specified.                   */
      if((jcprops->JPGWidth  < 1) || (jcprops->JPGWidth  > 65535) ||
         (jcprops->JPGHeight < 1) || (jcprops->JPGHeight > 65535))
      {
        TRACE1(trERROR,"ERROR: jcprops->JPGWidth is %d\n",jcprops->JPGWidth);
        TRACE1(trERROR,"ERROR: jcprops->JPGHeight is %d\n",jcprops->JPGHeight);
        TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
        return IJL_INVALID_JPEG_PROPERTIES;
      }

      /* Only allow the user to write JPEG images with          */
      /* channels <= the input number of DIB channels           */
      /* (except for IJL_G -> IJL_YCBCR which is 1 ch to 3 ch). */
      if((jcprops->DIBColor != IJL_G) && (jcprops->JPGColor != IJL_YCBCR))
      {
        if(jcprops->JPGChannels > jcprops->DIBChannels)
        {
          TRACE0(trERROR,"ERROR: jcprops->JPGChannels must be less or"
                        "equal that jcprops->DIBChannels");
          TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
          return IJL_INVALID_JPEG_PROPERTIES;
        }
      }
    }

    /* Check for valid JPEG sampling. */
    if((jcprops->JPGChannels == 1) && (jcprops->JPGSubsampling != IJL_NONE))
    {
      TRACE0(trERROR,"ERROR: unexpected JPEG subsampling\n");
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_UNSUPPORTED_SUBSAMPLING;
    }
    else if( (jcprops->JPGChannels == 3) &&
             ( (jcprops->JPGSubsampling != IJL_411) &&
               (jcprops->JPGSubsampling != IJL_422) &&
               (jcprops->JPGSubsampling != IJL_NONE) ) )
    {
      TRACE0(trERROR,"ERROR: unexpected JPEG subsampling\n");
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_UNSUPPORTED_SUBSAMPLING;
    }
    else if( (jcprops->JPGChannels == 4) &&
             ( (jcprops->JPGSubsampling != IJL_4114) &&
               (jcprops->JPGSubsampling != IJL_4224) &&
               (jcprops->JPGSubsampling != IJL_NONE) ) )
    {
      TRACE0(trERROR,"ERROR: unexpected JPEG subsampling\n");
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_UNSUPPORTED_SUBSAMPLING;
    }
    else if( ((jcprops->JPGChannels != 1) &&
              (jcprops->JPGChannels != 3) &&
              (jcprops->JPGChannels != 4) ) &&
             (jcprops->JPGSubsampling != IJL_NONE) )
    {
      TRACE0(trERROR,"ERROR: unexpected JPEG subsampling\n");
      TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
      return IJL_UNSUPPORTED_SUBSAMPLING;
    }
  }

  /* Check decoding color space options. */
  if(FALSE != read_test)
  {
    if((jcprops->JPGColor == IJL_G) && (jcprops->JPGChannels == 1))
    {
      if(((jcprops->DIBColor == IJL_G) && (jcprops->DIBChannels == 1)) ||
         ((jcprops->DIBColor == IJL_RGB) && (jcprops->DIBChannels == 3)) ||
         ((jcprops->DIBColor == IJL_BGR) && (jcprops->DIBChannels == 3)) ||
         ((jcprops->DIBColor == IJL_RGBA_FPX) && (jcprops->DIBChannels == 4)))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->JPGColor == IJL_RGB) && (jcprops->JPGChannels == 3))
    {
      if(((jcprops->DIBColor == IJL_RGB) && (jcprops->DIBChannels == 3)) ||
         ((jcprops->DIBColor == IJL_BGR) && (jcprops->DIBChannels == 3)) ||
         ((jcprops->DIBColor == IJL_RGBA_FPX) && (jcprops->DIBChannels == 4)))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->JPGColor == IJL_RGBA_FPX) && (jcprops->JPGChannels == 4))
    {
      if((jcprops->DIBColor == IJL_RGBA_FPX) && (jcprops->DIBChannels == 4))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->JPGColor == IJL_YCBCR) && (jcprops->JPGChannels == 3))
    {
      if(((jcprops->DIBColor == IJL_G) && (jcprops->DIBChannels == 1)) ||
         ((jcprops->DIBColor == IJL_RGB) && (jcprops->DIBChannels == 3)) ||
         ((jcprops->DIBColor == IJL_BGR) && (jcprops->DIBChannels == 3)) ||
         ((jcprops->DIBColor == IJL_RGBA_FPX) && (jcprops->DIBChannels == 4)))
      {
        color_combination = TRUE;
      }
      if(jcprops->DIBColor == IJL_YCBCR)
      {
        color_combination = TRUE; // OA: reading YCbCr 4:2:2 to YCbYCr format
      }

    }

    if((jcprops->JPGColor == IJL_YCBCRA_FPX) && (jcprops->JPGChannels == 4))
    {
      if((jcprops->DIBColor == IJL_RGBA_FPX) && (jcprops->DIBChannels == 4))
      {
        color_combination = TRUE;
      }
    }

    if(jcprops->JPGColor == IJL_OTHER)
    {
      if(jcprops->DIBColor == IJL_OTHER)
      {
        color_combination = TRUE;
      }
    }
  }
  else
  {
    /* Check encoding color space options. */

    if((jcprops->DIBColor == IJL_G) && (jcprops->DIBChannels == 1))
    {
      if(((jcprops->JPGColor == IJL_G) && (jcprops->JPGChannels == 1)) ||
         ((jcprops->JPGColor == IJL_YCBCR) && (jcprops->JPGChannels == 3)))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->DIBColor == IJL_RGB) && (jcprops->DIBChannels == 3))
    {
      if(((jcprops->JPGColor == IJL_G) && (jcprops->JPGChannels == 1)) ||
         ((jcprops->JPGColor == IJL_RGB) && (jcprops->JPGChannels == 3)) ||
         ((jcprops->JPGColor == IJL_YCBCR) && (jcprops->JPGChannels == 3)))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->DIBColor == IJL_RGB) && (jcprops->DIBChannels == 4))
    {
      if((jcprops->JPGColor == IJL_YCBCR) && (jcprops->JPGChannels == 3))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->DIBColor == IJL_BGR) && (jcprops->DIBChannels == 3))
    {
      if(((jcprops->JPGColor == IJL_G) && (jcprops->JPGChannels == 1)) ||
         ((jcprops->JPGColor == IJL_RGB) && (jcprops->JPGChannels == 3)) ||
         ((jcprops->JPGColor == IJL_YCBCR) && (jcprops->JPGChannels == 3)))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->DIBColor == IJL_RGBA_FPX) && (jcprops->DIBChannels == 4))
    {
      if(((jcprops->JPGColor == IJL_RGBA_FPX) && (jcprops->JPGChannels == 4)) ||
         ((jcprops->JPGColor == IJL_YCBCRA_FPX) && (jcprops->JPGChannels == 4)))
      {
        color_combination = TRUE;
      }
    }

    if((jcprops->DIBColor == IJL_YCBCR) && (jcprops->DIBChannels ==3) &&
       (jcprops->JPGColor == IJL_YCBCR) && (jcprops->JPGChannels ==3))
    {
      color_combination = TRUE;
    }

    if((jcprops->DIBColor == IJL_OTHER) && (jcprops->JPGChannels <= jcprops->DIBChannels))
    {
      if(jcprops->JPGColor == IJL_OTHER)
      {
        color_combination = TRUE;
      }
    }
  }

  if(FALSE == color_combination)
  {
    TRACE0(trERROR,"ERROR: wrong color/channels combination\n");
    TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
    return IJL_INVALID_JPEG_PROPERTIES;
  }

  /* Note, jquality is checked and bounded in Scale_Char_Matrix() */
  /* in Tables.cpp.  Resulting value is [1,100].                  */

  /* Check for valid enum for the DCT. */
  if(IJL_AAN != jprops->dcttype && IJL_IPP != jprops->dcttype)
  {
    TRACE0(trERROR,"ERROR: wrong DCT type\n");
    TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");
    return IJL_INVALID_JPEG_PROPERTIES;
  }

  if((jprops->upsampling_type == IJL_TRIANGLE_FILTER) && jprops->raw_coefs)
  {
    TRACE0(trERROR,"ERROR: wrong upsampling algorithm\n");
    return IJL_INVALID_JPEG_PROPERTIES;
  }

  TRACE0(trCALL|trAFTER,"leave CheckParameters()\n");

  return IJL_OK;
} /* CheckParameters() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    ijlRead
//
//  Purpose
//    To read JPEG data (entropy, or header, or both) into a
//    user-supplied buffer (to hold the data) and/or into the
//    JPEG_CORE_PROPERTIES structure (to hold the header info).
//
//  Context
//    See the user documentation for a detailed description on
//    the use of this function.  The jcprops main data members
//    are checked for consistency.  If you assert
//    UseJPEGPROPERTIES in JPEG_CORE_PROPERTIES, then
//    JPEG_PROPERTIES are used (to the exclusion of everything
//    in JPEG_CORE_PROPERTIES, and no data checking is done, so
//    BE CAREFUL!).
//
//  Returns
//    Any IJLERR value.
//    IJL_OK indicates success.
//
//  Parameters
//    jcprops  A pointer to an externally allocated
//             JPEG_CORE_PROPERTIES structure.
//    iotype   Specifies what type of read operation
//             is to be performed.
//
////////////////////////////////////////////////////////////////////////////
*F*/

IJLFUN(IJLERR) ijlRead(
  JPEG_CORE_PROPERTIES* jcprops,
  IJLIOTYPE             iotype)
{
  int          i;
  int          j;
  BOOL         bres;
  IJL_HANDLE   tempfile;
  IJLERR       jerr = IJL_OK;
  IJL_CONTEXT* ctx  = NULL;
  JPEG_PROPERTIES* jprops;

#if defined (_LINUX)
  sighandler_t old_fpe_handler;
  sighandler_t old_ill_handler;
  sighandler_t old_segv_handler;

  old_fpe_handler  = signal(SIGFPE,(sighandler_t)own_sig_fpe_ill_segv);
  old_ill_handler  = signal(SIGILL,(sighandler_t)own_sig_fpe_ill_segv);
  old_segv_handler = signal(SIGSEGV,(sighandler_t)own_sig_fpe_ill_segv);
#endif

  TRY_BEGIN

  TRACE0(trCALL|trBEFORE,"enter in ijlRead\n");

  jprops = &(jcprops->jprops);

  //if(jprops->processor_type != jprops->state.processor_type)
  //{
  //  ippStaticInitCpu(jprops->processor_type);
  //}

  jprops->iotype = iotype;

  TRACE1(trINFO,"iotype is %d\n",jprops->iotype);

  ctx = (IJL_CONTEXT*)jprops->state.ctx;

  ctx->pDCTCoefBuffer = NULL;

  jprops->state.u.pDecHuffState = (IppiDecodeHuffmanState*)ctx->pHuffStateBuf;

  jerr = CheckParameters(jcprops);

  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: Bad parameters\n");
    goto Exit;
  }

  if((jprops->iotype == IJL_JFILE_READHEADER) ||
     (jprops->iotype == IJL_JBUFF_READHEADER) ||
     (jprops->iotype == IJL_JFILE_READPARAMS) ||
     (jprops->iotype == IJL_JBUFF_READPARAMS) ||
     (jprops->iotype == IJL_JFILE_READTHUMBNAIL) ||
     (jprops->iotype == IJL_JBUFF_READTHUMBNAIL))
  {
    /* Reset the frame header indicator. */
    jprops->needframe = TRUE;
  }

  if((jprops->iotype == IJL_JFILE_READENTROPY) ||
     (jprops->iotype == IJL_JBUFF_READENTROPY) ||
     (jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
     (jprops->iotype == IJL_JBUFF_READWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
     (jprops->iotype == IJL_JBUFF_READONEEIGHTH) ||
     (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
     (jprops->iotype == IJL_JBUFF_READONEQUARTER) ||
     (jprops->iotype == IJL_JFILE_READONEHALF) ||
     (jprops->iotype == IJL_JBUFF_READONEHALF))
  {
    if(jprops->use_external_qtables)
    {
      for(i = 0; i < jprops->maxquantindex; i++)
      {
        jprops->jFmtQuant[i].ident     = jprops->rawquanttables[i].ident;
        jprops->jFmtQuant[i].precision = 0;
        for(j = 0; j < DCTSIZE2; j++)
        {
          jprops->jFmtQuant[i].elements[j] = jprops->rawquanttables[i].quantizer[j];
        }
      }
    }

    if(jprops->use_external_htables)
    {
      for(i = 0; i < jprops->maxhuffindex; i++)
      {
        int    type  = jprops->rawhufftables[i].hclass;
        int    ident = jprops->rawhufftables[i].ident;
        Ipp8u* bits  = jprops->rawhufftables[i].bits;
        Ipp8u* vals  = jprops->rawhufftables[i].vals;

        if(type)
        {
          jprops->jFmtAcHuffman[ident].ident = ident;
          jprops->jFmtAcHuffman[ident].huff_class = type;

          jerr = BuildDecoderHuffmanTable(bits,vals,&jprops->jFmtAcHuffman[ident]);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: BuildDecoderHuffmanTable() failed\n");
            jerr = IJL_BAD_HUFFMAN_TABLE;
            goto Exit;
          }
        }
        else
        {
          jprops->jFmtDcHuffman[ident].ident = ident;
          jprops->jFmtDcHuffman[ident].huff_class = type;

          jerr = BuildDecoderHuffmanTable(bits,vals,&jprops->jFmtDcHuffman[ident]);
          if(IJL_OK != jerr)
          {
            TRACE0(trERROR,"ERROR: BuildDecoderHuffmanTable() failed\n");
            jerr = IJL_BAD_HUFFMAN_TABLE;
            goto Exit;
          }
        }
      }
    }
  }
  else
  {
    /* Ignore default tables. */
    jcprops->jprops.nqtables      = 0;
    jcprops->jprops.maxquantindex = 0;
    jcprops->jprops.maxhuffindex  = 0;
    jcprops->jprops.nhuffActables = 0;
    jcprops->jprops.nhuffDctables = 0;
  }

  /* If the IJL has been accessed in file mode, and the desired */
  /* input file is not open, open it. */
  if((jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_READHEADER) ||
     (jprops->iotype == IJL_JFILE_READENTROPY) ||
     (jprops->iotype == IJL_JFILE_READPARAMS) ||
     (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
     (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
     (jprops->iotype == IJL_JFILE_READONEHALF) ||
     (jprops->iotype == IJL_JFILE_READTHUMBNAIL))
  {
    if(NULL == jprops->state.file)
    {
      tempfile = ownOpenFile(jcprops->JPGFile,OPEN_READ);

      if(NULL == tempfile)
      {
        TRACE1(trERROR,"ERROR: Can't open file %s\n",jcprops->JPGFile);
        jerr = IJL_INVALID_FILENAME;
        goto Exit;
      }

      jprops->state.file = tempfile;
    }
  }

  /* Copy JPEG_CORE_PROPERTIES parameters on top of JPEG_PROPERTIES. */
  if(jcprops->UseJPEGPROPERTIES == 0)
  {
    jprops->DIBBytes         = jcprops->DIBBytes;
    jprops->DIBWidth         = jcprops->DIBWidth;
    jprops->DIBHeight        = jcprops->DIBHeight;
    jprops->DIBPadBytes      = jcprops->DIBPadBytes;
    jprops->DIBChannels      = jcprops->DIBChannels;
    jprops->DIBColor         = jcprops->DIBColor;
    jprops->DIBSubsampling   = jcprops->DIBSubsampling;
    jprops->DIBLineBytes     = (jcprops->DIBWidth * jcprops->DIBChannels) +
                                  jcprops->DIBPadBytes;

    if(jprops->DIBHeight < 0)
    {
      jprops->DIBLineBytes = -jprops->DIBLineBytes;
    }

    jprops->JPGBytes         = jcprops->JPGBytes;
    jprops->JPGFile          = jcprops->JPGFile;
    jprops->JPGSizeBytes     = jcprops->JPGSizeBytes;
    jprops->JPGColor         = jcprops->JPGColor;

    jprops->cconversion_reqd = jcprops->cconversion_reqd;
    jprops->upsampling_reqd  = jcprops->upsampling_reqd;
    jprops->jquality         = jcprops->jquality;

    if(NULL == jprops->jscan)
    {
      jprops->state.unread_marker = 0;

      if((jprops->iotype != IJL_JFILE_READWHOLEIMAGE) &&
         (jprops->iotype != IJL_JFILE_READHEADER) &&
         (jprops->iotype != IJL_JFILE_READPARAMS) &&
         (jprops->iotype != IJL_JFILE_READENTROPY) &&
         (jprops->iotype != IJL_JFILE_READONEEIGHTH) &&
         (jprops->iotype != IJL_JFILE_READONEQUARTER) &&
         (jprops->iotype != IJL_JFILE_READONEHALF) &&
         (jprops->iotype != IJL_JFILE_READTHUMBNAIL))
      {
        /* Data to decode comes from a buffer. */
        jprops->state.start_entropy_ptr   = jprops->JPGBytes;
        jprops->state.end_entropy_ptr     = jprops->JPGBytes + jprops->JPGSizeBytes;
        jprops->state.cur_entropy_ptr     = jprops->JPGBytes;
        jprops->state.entropy_bytes_left  = jprops->JPGSizeBytes;
        jprops->state.entropy_buf_maxsize = jprops->JPGSizeBytes;
      }

      if((jprops->iotype == IJL_JFILE_READWHOLEIMAGE) ||
         (jprops->iotype == IJL_JFILE_READHEADER) ||
         (jprops->iotype == IJL_JFILE_READPARAMS) ||
         (jprops->iotype == IJL_JFILE_READENTROPY) ||
         (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
         (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
         (jprops->iotype == IJL_JFILE_READONEHALF) ||
         (jprops->iotype == IJL_JFILE_READTHUMBNAIL))
      {
        /* Input comes from a file. */
        jprops->state.start_entropy_ptr   = jprops->state.JPGBuffer;
        jprops->state.end_entropy_ptr     = jprops->state.JPGBuffer + JBUFSIZE;
        jprops->state.cur_entropy_ptr     = jprops->state.JPGBuffer;
        jprops->state.entropy_bytes_left  = JBUFSIZE;
        jprops->state.entropy_buf_maxsize = JBUFSIZE;
      }

      /* set file pointer to begin of file, */
      /* to find APP0 marker again          */
      if(IJL_JFILE_READTHUMBNAIL == jprops->iotype)
      {
        bres = ownSeekFile(jprops->state.file,0,FILE_BEGIN);

        if(FALSE == bres)
        {
          TRACE0(trERROR,"ERROR: seek in file error\n");
          goto Exit;
        }
      }

    } /* NULL == jprops->jscan */
  } /* jcprops->UseJPEGPROPERTIES == 0 */

  /* If the input DIB is bottom-up (instead of top-down), */
  /* set the DIB pointer to the last line of the image.   */
  if(jcprops->DIBHeight < 0)
  {
    jprops->DIBBytes += (jprops->DIBLineBytes * (jcprops->DIBHeight + 1));
  }

  /* Modify the following for ROI calculations. */
  jprops->state.DIB_ptr = jprops->DIBBytes;

  /* Qword align the MCU Buffer. */
  jprops->MCUBuf = (Ipp8u*)OWN_ALIGN_PTR(&jprops->tMCUBuf[0],CPU_CACHE_LINE);

  jerr = IJL_Decode(jprops);

  if(IJL_OK != jerr)
  {
    /* If the decode was halted by an interrupt, return with the appropriate */
    /* pixel coordinates in the right and bottom entries of the ROI. */
    if(IJL_INTERRUPT_OK == jerr)
    {
      /* Translate the MCU offsets into pixel offsets for the user. */
      jprops->roi.right =
        (jprops->roi.right + 1) * 8 * jprops->jframe.max_hsampling;

      if(jprops->roi.right > jprops->JPGWidth)
      {
        jprops->roi.right = jprops->JPGWidth;
      }

      jprops->roi.bottom =
        (jprops->roi.bottom + 1) * 8 * jprops->jframe.max_vsampling;

      if(jprops->roi.bottom > jprops->JPGHeight)
      {
        jprops->roi.bottom = jprops->JPGHeight;
      }
    }

    goto Exit;
  }

  if(jprops->SawAdobeMarker != 0)
  {
    if(jprops->JPGChannels == 1)
    {
      jprops->JPGColor = IJL_G;
    }

    if(jprops->JPGChannels == 3)
    {
      jprops->JPGColor = (jprops->AdobeXform != 0) ? IJL_YCBCR : IJL_RGB;
    }

    if(jprops->JPGChannels == 4)
    {
      jprops->DIBChannels = jprops->JPGChannels;

      jprops->JPGColor = (jprops->AdobeXform != 0) ? IJL_YCBCRA_FPX : IJL_RGBA_FPX;
      jprops->DIBColor = IJL_RGBA_FPX;
    }
  }
  else
  {
    if(jprops->JPGChannels == 1)
    {
      jprops->JPGColor = IJL_G;
    }
    /* JFIF proposal */
    if(jprops->jfif_app0_detected != 0)
    {
      if(jprops->JPGChannels == 3)
      {
        jprops->JPGColor = IJL_YCBCR;
      }
    }
  }

  if((jprops->DIBColor == IJL_YCBCR) && (jprops->JPGColor == IJL_YCBCR))
  {
    jprops->DIBSubsampling = IJL_422;
  }

  /* If the image was file based read in anything but "readwholeimage" */
  /* mode, we need to modify the current file pointer so that (if) we  */
  /* re-enter the IJL, we will be pointing to the correct location.    */
  if((jprops->iotype == IJL_JFILE_READHEADER) ||
     (jprops->iotype == IJL_JFILE_READENTROPY) ||
     (jprops->iotype == IJL_JFILE_READPARAMS) ||
     (jprops->iotype == IJL_JFILE_READONEEIGHTH) ||
     (jprops->iotype == IJL_JFILE_READONEQUARTER) ||
     (jprops->iotype == IJL_JFILE_READONEHALF) ||
     (jprops->iotype == IJL_JFILE_READTHUMBNAIL))
  {
    /* Entropy_bytes_left indicates the number of bytes left in the current */
    /* buffer. We need to seek 2 to the left of this position, because      */
    /* (along these code paths) we would have just seen a marker (2 bytes). */
    bres = ownSeekFile(
      jprops->state.file,
      (-(jprops->state.entropy_bytes_left) - ctx->scan_length),
      FILE_CURRENT);

    if(FALSE == bres)
    {
      TRACE0(trERROR,"ERROR: seek in file\n");
      jerr = IJL_FILE_ERROR;
      goto Exit;
    }
  }

  if((jprops->iotype == IJL_JBUFF_READPARAMS) ||
     (jprops->iotype == IJL_JBUFF_READTHUMBNAIL))
  {
    jprops->state.start_entropy_ptr = jprops->state.cur_entropy_ptr - ctx->scan_length;
  }

  /* Write the JPEG parameters to the JPEG_CORE_PROPERTIES. */
  jcprops->JPGWidth         = jprops->JPGWidth;
  jcprops->JPGHeight        = jprops->JPGHeight;
  jcprops->JPGChannels      = jprops->JPGChannels;

  jcprops->JPGThumbWidth    = jprops->JPGThumbWidth;
  jcprops->JPGThumbHeight   = jprops->JPGThumbHeight;

  jcprops->cconversion_reqd = jprops->cconversion_reqd;
  jcprops->upsampling_reqd  = jprops->upsampling_reqd;

  jcprops->JPGSubsampling = (IJL_JPGSUBSAMPLING)IJL_OTHER;

  if(jprops->jframe.ncomps == 1)
  {
    jcprops->JPGSubsampling = IJL_NONE;
  }
  else if(jprops->jframe.ncomps == 2)
  {
    if((jprops->jframe.comps[0].hsampling == 1) &&
       (jprops->jframe.comps[0].vsampling == 1) &&
       (jprops->jframe.comps[1].hsampling == 1) &&
       (jprops->jframe.comps[1].vsampling == 1))
    {
      jcprops->JPGSubsampling = IJL_NONE;
    }
  }
  else if(jprops->jframe.ncomps == 3)
  {
    if((jprops->jframe.comps[0].hsampling == 1) &&
       (jprops->jframe.comps[0].vsampling == 1) &&
       (jprops->jframe.comps[1].hsampling == 1) &&
       (jprops->jframe.comps[1].vsampling == 1) &&
       (jprops->jframe.comps[2].hsampling == 1) &&
       (jprops->jframe.comps[2].vsampling == 1))
    {
      jcprops->JPGSubsampling = IJL_NONE;
    }

    else if((jprops->jframe.comps[0].hsampling == 2) &&
            (jprops->jframe.comps[0].vsampling == 2) &&
            (jprops->jframe.comps[1].hsampling == 1) &&
            (jprops->jframe.comps[1].vsampling == 1) &&
            (jprops->jframe.comps[2].hsampling == 1) &&
            (jprops->jframe.comps[2].vsampling == 1))
    {
      jcprops->JPGSubsampling = IJL_411;
    }

    else if((jprops->jframe.comps[0].hsampling == 2) &&
            (jprops->jframe.comps[0].vsampling == 1) &&
            (jprops->jframe.comps[1].hsampling == 1) &&
            (jprops->jframe.comps[1].vsampling == 1) &&
            (jprops->jframe.comps[2].hsampling == 1) &&
            (jprops->jframe.comps[2].vsampling == 1))
    {
      jcprops->JPGSubsampling = IJL_422;
    }
  }
  else if(jprops->jframe.ncomps == 4)
  {
    if((jprops->jframe.comps[0].hsampling == 1) &&
       (jprops->jframe.comps[0].vsampling == 1) &&
       (jprops->jframe.comps[1].hsampling == 1) &&
       (jprops->jframe.comps[1].vsampling == 1) &&
       (jprops->jframe.comps[2].hsampling == 1) &&
       (jprops->jframe.comps[2].vsampling == 1) &&
       (jprops->jframe.comps[3].hsampling == 1) &&
       (jprops->jframe.comps[3].vsampling == 1))
    {
      jcprops->JPGSubsampling = IJL_NONE;
    }

    else if((jprops->jframe.comps[0].hsampling == 2) &&
            (jprops->jframe.comps[0].vsampling == 2) &&
            (jprops->jframe.comps[1].hsampling == 1) &&
            (jprops->jframe.comps[1].vsampling == 1) &&
            (jprops->jframe.comps[2].hsampling == 1) &&
            (jprops->jframe.comps[2].vsampling == 1) &&
            (jprops->jframe.comps[3].hsampling == 2) &&
            (jprops->jframe.comps[3].vsampling == 2))
    {
      jcprops->JPGSubsampling = IJL_4114;
    }

    else if((jprops->jframe.comps[0].hsampling == 2) &&
            (jprops->jframe.comps[0].vsampling == 1) &&
            (jprops->jframe.comps[1].hsampling == 1) &&
            (jprops->jframe.comps[1].vsampling == 1) &&
            (jprops->jframe.comps[2].hsampling == 1) &&
            (jprops->jframe.comps[2].vsampling == 1) &&
            (jprops->jframe.comps[3].hsampling == 2) &&
            (jprops->jframe.comps[3].vsampling == 1))
    {
      jcprops->JPGSubsampling = IJL_4224;
    }
  }

  jprops->JPGSubsampling = jcprops->JPGSubsampling;

  jcprops->JPGColor      = jprops->JPGColor;
  jcprops->DIBColor      = jprops->DIBColor;
  jcprops->DIBChannels   = jprops->DIBChannels;

  /* Close the output file, if it exists. */
  if(IJL_JFILE_READWHOLEIMAGE == jprops->iotype ||
     IJL_JFILE_READHEADER     == jprops->iotype ||
     IJL_JFILE_READENTROPY    == jprops->iotype)
  {
    if(NULL != jprops->state.file)
    {
      bres = ownCloseFile(jprops->state.file);
      if(FALSE == bres)
      {
        TRACE1(trERROR,"ERROR: Can't close file %s\n",jcprops->JPGFile);
        jerr = IJL_ERR_FILECLOSE;
        goto Exit;
      }

      jprops->state.file = NULL;
    }
  }

  TRY_END

  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: EXCEPTION detected in ijlRead\n");
  jerr = IJL_EXCEPTION_DETECTED;

  CATCH_END


#if defined (_LINUX)
  /* restore signal handlers */
  signal(SIGFPE,(sighandler_t)old_fpe_handler);
  signal(SIGILL,(sighandler_t)old_ill_handler);
  signal(SIGSEGV,(sighandler_t)old_segv_handler);
#endif

Exit:

  if(IJL_OK > jerr)
  {
    if(NULL != jcprops->jprops.state.file)
    {
      ownCloseFile(jcprops->jprops.state.file);
      jcprops->jprops.state.file = NULL;
    }
  }

  TRACE0(trCALL|trAFTER,"leave ijlRead\n");

  return jerr;
} /* ijlRead() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    ijlWrite
//
//  Purpose
//    To write JPEG data (entropy, or header, or both) into a
//    user-supplied buffer (to hold the data) and/or into the
//    JPEG_CORE_PROPERTIES structure (to hold the header info).
//
//  Context
//    See the user documentation for a detailed description on
//    the use of this function.  The jcprops main data members
//    are checked for consistency.  If you assert
//    UseJPEGPROPERTIES in JPEG_CORE_PROPERTIES, then
//    JPEG_PROPERTIES are used (to the exclusion of everything
//    in JPEG_CORE_PROPERTIES, and no data checking is done, so
//    BE CAREFUL!).
//
//  Returns
//    Any IJLERR value.
//    IJL_OK indicates success.
//
//  Parameters
//    jcprops  A pointer to an externally allocated
//             JPEG_CORE_PROPERTIES structure.
//    iotype   Specifies what type of write operation
//             is to be performed.
//
////////////////////////////////////////////////////////////////////////////
*F*/

IJLFUN(IJLERR) ijlWrite(
  JPEG_CORE_PROPERTIES* jcprops,
  IJLIOTYPE             iotype)
{
  int        i;
  BOOL       bres;
  int        bytes_written;
  IJLERR     jerr = IJL_OK;
  IJL_HANDLE tempfile = NULL;
  IJL_CONTEXT* ctx;
  JPEG_PROPERTIES* jprops;

#if defined (_LINUX)
  sighandler_t old_fpe_handler;
  sighandler_t old_ill_handler;
  sighandler_t old_segv_handler;

  old_fpe_handler  = signal(SIGFPE,(sighandler_t)own_sig_fpe_ill_segv);
  old_ill_handler  = signal(SIGILL,(sighandler_t)own_sig_fpe_ill_segv);
  old_segv_handler = signal(SIGSEGV,(sighandler_t)own_sig_fpe_ill_segv);
#endif

  TRY_BEGIN

  TRACE0(trCALL|trBEFORE,"enter in ijlWrite\n");

  jprops = &jcprops->jprops;

  //if(jprops->processor_type != jprops->state.processor_type)
  //{
  //  ippStaticInitCpu(jprops->processor_type);
  //}

  ctx = (IJL_CONTEXT*)jprops->state.ctx;

  jprops->state.u.pEncHuffState = (IppiEncodeHuffmanState*)ctx->pHuffStateBuf;

  bytes_written = 0;

  jprops->iotype = iotype;

  jerr = CheckParameters(jcprops);

  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: Bad parameters\n");
    goto Exit;
  }

  if((jprops->iotype == IJL_JFILE_WRITEWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_WRITEHEADER) ||
     (jprops->iotype == IJL_JFILE_WRITEENTROPY))
  {
    if(NULL == jprops->state.file)
    {
      tempfile = ownOpenFile(jcprops->JPGFile,OPEN_READ_WRITE);
      if(NULL == tempfile)
      {
        TRACE1(trERROR,"ERROR: Can't open file %s\n",jcprops->JPGFile);
        jerr = IJL_INVALID_FILENAME;
        goto Exit;
      }

      jprops->state.file = tempfile;
    }
  }

  /* Copy JPEG_CORE_PROPERTIES parameters on top of JPEG_PROPERTIES. */
  if(jcprops->UseJPEGPROPERTIES == 0 && jprops->jscan == 0)
  {
    jprops->JPGWidth         = jcprops->JPGWidth;
    jprops->JPGHeight        = jcprops->JPGHeight;
    jprops->JPGChannels      = jcprops->JPGChannels;
    jprops->JPGColor         = jcprops->JPGColor;
    jprops->JPGSubsampling   = jcprops->JPGSubsampling;
    jprops->JPGBytes         = jcprops->JPGBytes;
    jprops->JPGSizeBytes     = jcprops->JPGSizeBytes;
    jprops->JPGFile          = jcprops->JPGFile;

    jprops->DIBBytes         = jcprops->DIBBytes;
    jprops->DIBWidth         = jcprops->DIBWidth;
    jprops->DIBHeight        = jcprops->DIBHeight;
    jprops->DIBPadBytes      = jcprops->DIBPadBytes;
    jprops->DIBChannels      = jcprops->DIBChannels;
    jprops->DIBColor         = jcprops->DIBColor;
    jprops->DIBSubsampling   = jcprops->DIBSubsampling;
    jprops->DIBLineBytes     = (jprops->DIBSubsampling == IJL_422) ?
                               (jcprops->DIBWidth*2)                   +jcprops->DIBPadBytes:
                               (jcprops->DIBWidth*jcprops->DIBChannels)+jcprops->DIBPadBytes;

    if(jprops->DIBHeight < 0)
    {
      jprops->DIBLineBytes = -jprops->DIBLineBytes;
    }

    jprops->cconversion_reqd = jcprops->cconversion_reqd;
    jprops->upsampling_reqd  = jcprops->upsampling_reqd;
    jprops->jquality         = jcprops->jquality;

    if((jprops->DIBSubsampling == IJL_422) &&
       (jprops->DIBColor == IJL_YCBCR) &&
       (jprops->JPGColor == IJL_YCBCR))
    {
      jprops->JPGSubsampling = IJL_422;
    }
  }

  /* set default tables if havn't user supplied tables */
  jerr = SetDefaultTables(jprops);
  if(IJL_OK != jerr)
  {
    TRACE0(trERROR,"ERROR: SetDefaultTables() failed!\n");
    goto Exit;
  }

  /* Determine if CC is required, and set cconversion_reqd. */
  jprops->cconversion_reqd = TRUE;

  if((jprops->JPGColor == IJL_RGB) ||
     (jprops->JPGColor == IJL_RGBA_FPX) ||
     (jprops->DIBColor == IJL_G) ||
     (jprops->DIBColor == IJL_OTHER) ||
     (jprops->DIBColor == IJL_YCBCR))
  {
    jprops->cconversion_reqd = FALSE;
  }

  jprops->jframe.ncomps = jcprops->JPGChannels;


  /* Do parameter checking for subsampling here. */
  switch(jcprops->JPGSubsampling)
  {
  case IJL_NONE:
    for(i = 0; i < jprops->jframe.ncomps; i++)
    {
      jprops->jframe.comps[i].ident     = i;
      jprops->jframe.comps[i].hsampling = 1;
      jprops->jframe.comps[i].vsampling = 1;
    }

    if(FALSE == jprops->use_external_qtables)
    {
      /* Set the quantization table selector to default values. */
      jprops->jframe.comps[0].quant_sel = 0;
      if((jprops->cconversion_reqd) && (jprops->jframe.ncomps >= 2))
      {
        for(i = 1; i < jprops->jframe.ncomps; i++)
        {
          jprops->jframe.comps[i].quant_sel = 1;
        }
      }
    }

    break;

  case IJL_411:
    /* 1st component */
    jprops->jframe.comps[0].ident     = 0;
    jprops->jframe.comps[0].hsampling = 2;
    jprops->jframe.comps[0].vsampling = 2;
    /* 2st component */
    jprops->jframe.comps[1].ident     = 1;
    jprops->jframe.comps[1].hsampling = 1;
    jprops->jframe.comps[1].vsampling = 1;
    /* 3st component */
    jprops->jframe.comps[2].ident     = 2;
    jprops->jframe.comps[2].hsampling = 1;
    jprops->jframe.comps[2].vsampling = 1;

    if(FALSE == jprops->use_external_qtables)
    {
      /* Set the quantization table selector to default values. */
      if(jcprops->cconversion_reqd)
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 1;
        jprops->jframe.comps[2].quant_sel = 1;
      }
      else
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 0;
        jprops->jframe.comps[2].quant_sel = 0;
      }
    }

    break;

  case IJL_422:
    /* 1st component */
    jprops->jframe.comps[0].ident     = 0;
    jprops->jframe.comps[0].hsampling = 2;
    jprops->jframe.comps[0].vsampling = 1;
    /* 2st component */
    jprops->jframe.comps[1].ident     = 1;
    jprops->jframe.comps[1].hsampling = 1;
    jprops->jframe.comps[1].vsampling = 1;
    /* 3st component */
    jprops->jframe.comps[2].ident     = 2;
    jprops->jframe.comps[2].hsampling = 1;
    jprops->jframe.comps[2].vsampling = 1;

    if(FALSE == jprops->use_external_qtables)
    {
      /* Set the quantization table selector to default values. */
      if(jcprops->cconversion_reqd)
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 1;
        jprops->jframe.comps[2].quant_sel = 1;
      }
      else
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 0;
        jprops->jframe.comps[2].quant_sel = 0;
      }
    }

    break;

  case IJL_4114:
    /* 1st component */
    jprops->jframe.comps[0].ident     = 0;
    jprops->jframe.comps[0].hsampling = 2;
    jprops->jframe.comps[0].vsampling = 2;
    /* 2st component */
    jprops->jframe.comps[1].ident     = 1;
    jprops->jframe.comps[1].hsampling = 1;
    jprops->jframe.comps[1].vsampling = 1;
    /* 3st component */
    jprops->jframe.comps[2].ident     = 2;
    jprops->jframe.comps[2].hsampling = 1;
    jprops->jframe.comps[2].vsampling = 1;
    /* 4st component */
    jprops->jframe.comps[3].ident     = 3;
    jprops->jframe.comps[3].hsampling = 2;
    jprops->jframe.comps[3].vsampling = 2;

    if(FALSE == jprops->use_external_qtables)
    {
      /* Set the quantization table selector to default values. */
      if(jcprops->cconversion_reqd)
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 1;
        jprops->jframe.comps[2].quant_sel = 1;
        jprops->jframe.comps[3].quant_sel = 0;
      }
      else
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 0;
        jprops->jframe.comps[2].quant_sel = 0;
        jprops->jframe.comps[3].quant_sel = 0;
      }
    }

    break;

  case IJL_4224:
    /* 1st component */
    jprops->jframe.comps[0].ident     = 0;
    jprops->jframe.comps[0].hsampling = 2;
    jprops->jframe.comps[0].vsampling = 1;
    /* 2st component */
    jprops->jframe.comps[1].ident     = 1;
    jprops->jframe.comps[1].hsampling = 1;
    jprops->jframe.comps[1].vsampling = 1;
    /* 3st component */
    jprops->jframe.comps[2].ident     = 2;
    jprops->jframe.comps[2].hsampling = 1;
    jprops->jframe.comps[2].vsampling = 1;
    /* 4st component */
    jprops->jframe.comps[3].ident     = 3;
    jprops->jframe.comps[3].hsampling = 2;
    jprops->jframe.comps[3].vsampling = 1;

    if(FALSE == jprops->use_external_qtables)
    {
      /* Set the quantization table selector to default values. */
      if(jcprops->cconversion_reqd)
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 1;
        jprops->jframe.comps[2].quant_sel = 1;
        jprops->jframe.comps[3].quant_sel = 0;
      }
      else
      {
        jprops->jframe.comps[0].quant_sel = 0;
        jprops->jframe.comps[1].quant_sel = 0;
        jprops->jframe.comps[2].quant_sel = 0;
        jprops->jframe.comps[3].quant_sel = 0;
      }
    }

    break;

  /* Bypass forced conversions */
  default:
    TRACE1(trWARN,"Unexpected JPGSubsampling 0x%08X\n",
      jcprops->JPGSubsampling);

    break;

  } /* end of switch(jcprops->JPGSubsampling) */

  /* Defaults: */
  jprops->jframe.precision = 8;
  jprops->jframe.width     = jcprops->JPGWidth;
  jprops->jframe.height    = jcprops->JPGHeight;

  /* 1. Determine if US is required, and set upsampling_reqd. */
  /* 2. Determine the max h and v sampling values.            */
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

    if((jprops->jframe.comps[i].hsampling != 1) ||
       (jprops->jframe.comps[i].vsampling != 1))
    {
      jprops->upsampling_reqd = TRUE;
    }
  }

  if(jprops->jinterleaveType == 1)
  {
    jprops->jframe.MCUwidth  = 8;
    jprops->jframe.MCUheight = 8;
  }
  else
  {
    jprops->jframe.MCUwidth  = 8 * jprops->jframe.max_hsampling;
    jprops->jframe.MCUheight = 8 * jprops->jframe.max_vsampling;
  }

  /* Determine the number of MCUs in horizontal and vertical directions. */
  jprops->numxMCUs = (jprops->JPGWidth + (jprops->jframe.MCUwidth - 1)) /
    (jprops->jframe.MCUwidth);

  jprops->numyMCUs = (jprops->JPGHeight + (jprops->jframe.MCUheight - 1)) /
    (jprops->jframe.MCUheight);

  jprops->jframe.horMCU   = jprops->numxMCUs;
  jprops->jframe.totalMCU = jprops->numxMCUs * jprops->numyMCUs;

  if(NULL == jprops->jscan)
  {
    jprops->state.entropy_bytes_processed = 0;

    /* if the input DIB is bottom-up (instead of top-down), */
    /* set the DIB pointer to the last line of the image.   */
    if(jcprops->DIBHeight < 0)
    {
      jprops->DIBBytes += (jprops->DIBLineBytes * (jcprops->DIBHeight + 1));
    }

    jprops->state.DIB_ptr = jprops->DIBBytes;

    jerr = Encode_Frame_Init(jprops);

    if(IJL_OK != jerr)
    {
      TRACE0(trERROR,"ERROR: Encode_Frame_Init() failed");
      goto Exit;
    }

    if((jprops->iotype == IJL_JFILE_WRITEHEADER) ||
       (jprops->iotype == IJL_JBUFF_WRITEHEADER))
    {
      jprops->JPGSizeBytes  = jprops->state.entropy_bytes_processed;
      jcprops->JPGSizeBytes = jprops->JPGSizeBytes;
    }
  } /* if(NULL == jprops->jscan) */

  if((jprops->iotype == IJL_JFILE_WRITEWHOLEIMAGE) ||
     (jprops->iotype == IJL_JBUFF_WRITEWHOLEIMAGE) ||
     (jprops->iotype == IJL_JFILE_WRITEENTROPY)    ||
     (jprops->iotype == IJL_JBUFF_WRITEENTROPY))
  {
    /* 32 byte (cache line) align the MCU Buffer. */
    jprops->MCUBuf = (Ipp8u*)OWN_ALIGN_PTR(&jprops->tMCUBuf[0],CPU_CACHE_LINE);

    if(jprops->progressive_found == 0)
    {
      jerr = Encode_Frame_Baseline(jprops);
    }
    else
    {
      jerr = Encode_Frame_Progressive(jprops);
    }

    jprops->needframe = TRUE;

    bytes_written += jprops->state.entropy_bytes_processed;

    jprops->JPGSizeBytes  = bytes_written;
    jcprops->JPGSizeBytes = bytes_written;
  }

  /* If the decode was halted by an interrupt, return with the          */
  /* appropriate pixel coordinates in the right and bottom ROI entries. */
  if(jerr == IJL_INTERRUPT_OK || jerr == IJL_ROI_OK)
  {
    /* Translate the MCU offsets into pixel offsets for the user. */
    jprops->roi.right = (jprops->roi.right + 1) * 8 * jprops->jframe.max_hsampling;

    if(jprops->roi.right > jprops->JPGWidth)
    {
      jprops->roi.right = jprops->JPGWidth;
    }

    jprops->roi.bottom = (jprops->roi.bottom + 1) * 8 * jprops->jframe.max_vsampling;

    if(jprops->roi.bottom > jprops->JPGHeight)
    {
      jprops->roi.bottom = jprops->JPGHeight;
    }

    goto Exit;
  }

  /* Close the output file, if it exists.                             */
  /* This probably shouldn't be done every time.                      */
  /* The user should have control whether or not to close the file at */
  /* the end of an operation.                                         */
  if(NULL != jprops->state.file)
  {
    bres = ownCloseFile(jprops->state.file);
    if(FALSE == bres)
    {
      TRACE1(trERROR,"ERROR: Can't close file %s\n",jcprops->JPGFile);
      jerr = IJL_ERR_FILECLOSE;
      goto Exit;
    }
    jprops->state.file = NULL;
  }

  TRY_END

  CATCH_BEGIN

  TRACE0(trERROR,"ERROR: EXCEPTION detected in ijlWrite\n");
  jerr = IJL_EXCEPTION_DETECTED;

  CATCH_END

Exit:

#if defined (_LINUX)
  /* restore signal handlers */
  signal(SIGFPE,(sighandler_t)old_fpe_handler);
  signal(SIGILL,(sighandler_t)old_ill_handler);
  signal(SIGSEGV,(sighandler_t)old_segv_handler);
#endif

  if(IJL_OK > jerr)
  {
    if(NULL != jcprops->jprops.state.file)
    {
      ownCloseFile(jcprops->jprops.state.file);
      jcprops->jprops.state.file = NULL;
    }
  }

  TRACE0(trCALL|trAFTER,"leave ijlWrite\n");

  return jerr;
} /* ijlWrite() */



/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    ijlGetLibVersion
//
//  Purpose
//    To identify the version number of the IJL.
//
//  Context
//    Call to get the IJL version number.
//
//  Returns
//    pointer to IJLLibVersion struct
//
//  Parameters
//    none
//
////////////////////////////////////////////////////////////////////////////
*F*/

IJLFUN(const IJLibVersion*) ijlGetLibVersion(
  void)
{
  /* Return the IJL version info. */
  TRACE0(trCALL|trBEFORE,"enter in ijlGetLibVersion\n");
  TRACE0(trCALL|trAFTER,"leave ijlGetLibVersion\n");
  return ownGetLibVersion();
} /* ijlGetLibVersion() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    ijlErrorStr
//
//  Purpose
//    Gets the string to describe error code.
//
//  Context
//    Is called to get descriptive string on arbitrary IJLERR code.
//
//  Returns
//    pointer to string
//
// Parameters
//    IJLERR - IJL error code
//
////////////////////////////////////////////////////////////////////////////
*F*/

IJLFUN(LPCSTR) ijlErrorStr(
  IJLERR code)
{
  LPCSTR string;

  TRACE0(trCALL|trBEFORE,"enter in ijlErrorStr\n");
  TRACE1(trINFO,"error code - 0x%08X\n",code);

  switch(code)
  {
  case IJL_OK:                           string = "Success"; break;
  case IJL_INTERRUPT_OK:                 string = "Interrupt Success"; break;
  case IJL_ROI_OK:                       string = "ROI Success";    break;

  case IJL_EXCEPTION_DETECTED:           string = "Exception detected";    break;
  case IJL_INVALID_ENCODER:              string = "Invalid Encoder";    break;
  case IJL_UNSUPPORTED_SUBSAMPLING:      string = "Unsupported subsampling";    break;
  case IJL_UNSUPPORTED_BYTES_PER_PIXEL:  string = "Unsupported bytes per pixel";    break;
  case IJL_MEMORY_ERROR:                 string = "Memory error";    break;
  case IJL_BAD_HUFFMAN_TABLE:            string = "Bad Huffman table";    break;
  case IJL_BAD_QUANT_TABLE:              string = "Bad Quantization table";    break;
  case IJL_INVALID_JPEG_PROPERTIES:      string = "Invalid JPEG_PROPERTIES";    break;
  case IJL_ERR_FILECLOSE:                string = "Error close file";    break;
  case IJL_INVALID_FILENAME:             string = "Invalid file name";    break;
  case IJL_ERROR_EOF:                    string = "Error EOF";    break;
  case IJL_PROG_NOT_SUPPORTED:           string = "Not supported";    break;
  case IJL_ERR_NOT_JPEG:                 string = "Not JPEG";    break;
  case IJL_ERR_COMP:                     string = "Error COMP";    break;
  case IJL_ERR_SOF:                      string = "Error SOF";    break;
  case IJL_ERR_DNL:                      string = "Error DNL";    break;
  case IJL_ERR_NO_HUF:                   string = "No Huffman table";    break;
  case IJL_ERR_NO_QUAN:                  string = "No Quantization table";    break;
  case IJL_ERR_NO_FRAME:                 string = "No frame";    break;
  case IJL_ERR_MULT_FRAME:               string = "Multiply frame";    break;
  case IJL_ERR_DATA:                     string = "Data error";    break;
  case IJL_ERR_NO_IMAGE:                 string = "No image";    break;
  case IJL_FILE_ERROR:                   string = "File error";    break;
  case IJL_INTERNAL_ERROR:               string = "Internal error";    break;
  case IJL_BAD_RST_MARKER:               string = "Bad RST marker";    break;
  case IJL_THUMBNAIL_DIB_TOO_SMALL:      string = "Thumbnail too small";    break;
  case IJL_THUMBNAIL_DIB_WRONG_COLOR:    string = "Thumbnail has wrong color";    break;
  case IJL_BUFFER_TOO_SMALL:             string = "Output buffer too small";    break;
  case IJL_UNSUPPORTED_FRAME:            string = "Unsupported frame";    break;
  case IJL_ERR_COM_BUFFER:               string = "Error access to jpeg_comment buffer";    break;
  case IJL_RESERVED:                     string = "Reserved";    break;
  default:                               string = "Unknown error code";    break;
  }

  TRACE0(trCALL|trAFTER,"leave ijlErrorStr\n");

  return string;
} /* ijlErrorStr() */

