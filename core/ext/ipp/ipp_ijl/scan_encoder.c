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
#ifndef __SCAN_ENCODER_H__
#include "scan_manager/scan_encoder.h"
#endif


OWNFUN(IJLERR) Fill_coeffs_buffer(
  JPEG_PROPERTIES* jprops)
{
  int     i;
  int     j;
  int     k;
  int     l;
  int     v;
  int     h;
  int     jj;
  int     bytes_per_block;
  int     bytes_per_mcu;
  int     blocks_per_mcu;
  int     blocks_per_component[4];
  FRAME*  pFrame;
  Ipp16s* MCUptr;
  Ipp16s* coef_buf;
  Ipp16u* q_table[4];
  IJL_CONTEXT* ctx;
  IJLERR jerr = IJL_OK;

  ctx = (IJL_CONTEXT*)jprops->state.ctx;

  pFrame = &jprops->jframe;

  coef_buf = jprops->coef_buffer;

  for(k = 0, blocks_per_mcu = 0; k < pFrame->ncomps; k++)
  {
    blocks_per_component[k] = pFrame->comps[k].vsampling *
                              pFrame->comps[k].hsampling;

    blocks_per_mcu += blocks_per_component[k];

    q_table[k] = jprops->jEncFmtQuant[pFrame->comps[k].quant_sel];
  }

  bytes_per_block = DCTSIZE2 * sizeof(Ipp16s);
  bytes_per_mcu = bytes_per_block * blocks_per_mcu;

  for(j = 0; j < jprops->numyMCUs; j++)
  {
    for(i = 0; i < jprops->numxMCUs; i++)
    {
      /* Re-set MCU buffer pointer */
      MCUptr = (Ipp16s*)jprops->MCUBuf;

      /* Perform color conversion/subsampling on the whole MCU */
      if(!jprops->raw_coefs)
      {
        /* raw sampled data */
        ctx->__g_cc_and_ss_mcu(jprops,i,j);
      }
      else
      {
        for(k = 0; k < pFrame->ncomps; k++)
        {
          /* vertical blocks in MCU */
          for(v = 0; v < pFrame->comps[k].vsampling; v++)
          {
            /* horizontal blocks in MCU */
            for(h = 0; h < pFrame->comps[k].hsampling; h++)
            {

              unsigned short* cvPtr = jprops->raw_coefs->raw_ptrs[k] +
              (j * jprops->numxMCUs * pFrame->comps[k].hsampling *
                                      pFrame->comps[k].vsampling * DCTSIZE2) +
              (v * DCTSIZE2 * pFrame->comps[k].hsampling * jprops->numxMCUs) +
              (i * pFrame->comps[k].hsampling * 8) + h * 8;
              for(jj = 0; jj < 8; jj++, cvPtr += (jprops->numxMCUs * pFrame->comps[k].hsampling * 8), MCUptr += 8)
              {
                IPPCALL(ippsCopy_8u)((const Ipp8u*)cvPtr,(Ipp8u*)MCUptr,sizeof(Ipp16s) << 3);
              }
            } /* pFrame->comps[k].hsampling */
          } /* pFrame->comps[k].vsampling */
        } /* pFrame->ncomps */

        MCUptr = (Ipp16s*)jprops->MCUBuf;

        if(jprops->raw_coefs->data_type)
        {
          for(k = blocks_per_mcu * DCTSIZE2; k--; MCUptr++)
          {
            *MCUptr -= 128;
          }
          MCUptr = (Ipp16s*)jprops->MCUBuf;
        }
      }

      /* Perform forward DCTs on the whole MCU */
      for(k = 0; k < blocks_per_mcu; k++)
      {
        fDCT8x8(MCUptr + (k << 6));
      }

      for(k = 0; k < pFrame->ncomps; k++)
      {
        for(l = 0; l < blocks_per_component[k]; l++, MCUptr += DCTSIZE2)
        {
          /* quantize */
          fQnt8x8(MCUptr,q_table[k]);
        }
      }

      /* copy to persistent coefficient buffer */
      IPPCALL(ippsCopy_8u)(jprops->MCUBuf,(Ipp8u*)coef_buf,bytes_per_mcu);
      coef_buf += (DCTSIZE2 * blocks_per_mcu);
    } /* for numxMCUs */
  } /* for numyMCUs */

  return jerr;
} /* Fill_coeffs_buffer() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    Gather_statistics
//
//  Purpose
//
//  Context
//
//  Returns
//    Valid error code, or 0 if OK.
//
//  Parameters
//    jprops  The pointer to IJL variables structure.
//    pScan   The pointer to current scan variables.
//    FREQ    The array of huffman symbols statistics
//    pState  The pointer to IJL state variables.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) Gather_statistics(
  JPEG_PROPERTIES* jprops,
  SCAN*            pScan,
  int              FREQ[2][256],
  STATE*           pState)
{
  int     i, j;
  int     k, l;
  int     m, n;
  int     idx;
  int     comp_id;
  int     mcu_size;
  int     mcu_row_size;
  int     blocks_per_mcu;
  int     blocks_per_component[4];
  int     offset;
  Ipp16s* coef_buf;
  IJLERR  jerr = IJL_OK;

  TRACE0(trCALL|trBEFORE,"enter in Gather_statistics()\n");

  for(k = 0, blocks_per_mcu = 0; k < jprops->jframe.ncomps; k++)
  {
    blocks_per_component[k] = jprops->jframe.comps[k].vsampling *
                              jprops->jframe.comps[k].hsampling;

    blocks_per_mcu += blocks_per_component[k];
  }

  mcu_size     = blocks_per_mcu * DCTSIZE2;
  mcu_row_size = mcu_size * jprops->numxMCUs;

  if(pScan->start_spec == 0 && pScan->end_spec == 0)
  {
    /* gather DC scan statistics */
    if(pScan->approx_high == 0)
    {
      for(j = 0; j < jprops->numyMCUs; j++)
      {
        for(i = 0; i < jprops->numxMCUs; i++)
        {
          coef_buf = jprops->coef_buffer + j * mcu_row_size + i * mcu_size;

          for(k = 0; k < pScan->ncomps; k++)
          {
            for(l = 0; l < blocks_per_component[k]; l++)
            {
              jerr = Gather_Prog_DC_SA(
                coef_buf,
                (Ipp16s*)&pScan->dc_diff[k],
                pScan->approx_low,
                FREQ[jprops->HuffIdentifierDC[k]]);
              if(IJL_OK != jerr)
                goto Exit;
              coef_buf += DCTSIZE2;
            } /* for block in components */
          } /* for components */
        } /* for numxMCUs */
      } /* for numyMCUs */
    } /* pScan->approx_high == 0 */
  }
  else
  {
    comp_id = pScan->comps->comp;

    idx = jprops->HuffIdentifierAC[comp_id];

    for(i = 0, offset = 0; i < comp_id; i++)
    {
      offset += blocks_per_component[i];
    }

    /* gather AC scan statistics */
    for(j = 0; j < jprops->numyMCUs; j++)
    {
      for(m = 0; m < pScan->comps->vsampling; m++)
      {
        /* Skip the last block row of the image if greater than the image height. */
        if(((j*pScan->comps[0].vsampling*8) + (m*8)) >= jprops->JPGHeight)
        {
          break;
        }

        for(i = 0; i < jprops->numxMCUs; i++)
        {
          coef_buf = jprops->coef_buffer + j * mcu_row_size + i * mcu_size;
          coef_buf += (offset * DCTSIZE2 + m * pScan->comps->vsampling * DCTSIZE2);

          for(n = 0; n < pScan->comps->hsampling; n++)
          {
            /* Ignore the last column(s) of the image. */
            if(((i*pScan->comps[0].hsampling*8) + (n*8)) >= jprops->JPGWidth)
            {
              break;
            }

            if(pScan->approx_high == 0)
            {
              jerr = Gather_Prog_AC_first(
                coef_buf,
                pScan->start_spec,
                pScan->end_spec,
                pScan->approx_low,
                FREQ[idx],
                pState,
                0);
              if(IJL_OK != jerr)
                goto Exit;
            }
            else
            {
              jerr = Gather_Prog_AC_refine(
                coef_buf,
                pScan->start_spec,
                pScan->end_spec,
                pScan->approx_low,
                FREQ[idx],
                pState,
                0);
              if(IJL_OK != jerr)
                goto Exit;
            } /* Ah */
            coef_buf += DCTSIZE2;
          } /* pScan->comps->hsampling */
        } /* for numxMCUs */
      } /* pScan->comps->vsampling */
    } /* for numyMCUs */

    /* Flush huffman state */
    if(pScan->approx_high == 0)
    {
      jerr = Gather_Prog_AC_first(
        0,
        pScan->start_spec,
        pScan->end_spec,
        pScan->approx_low,
        FREQ[idx],
        pState,
        1);
      if(IJL_OK != jerr)
        goto Exit;
    }
    else
    {
      jerr = Gather_Prog_AC_refine(
        0,
        pScan->start_spec,
        pScan->end_spec,
        pScan->approx_low,
        FREQ[idx],
        pState,
        1);
      if(IJL_OK != jerr)
        goto Exit;
    } /* Ah */
  } /* if(pScan->start_spec == 0 && pScan->end_spec == 0) */

Exit:

  TRACE0(trCALL|trAFTER,"leave Gather_statistics()\n");

  return jerr;
} /* Gather_statistics() */


/*F*
////////////////////////////////////////////////////////////////////////////
// Name
//   EncodeScanBaseline
//
// Purpose
//   Encode a scan of data with restart markers.
//   Currently the scan does not include restart markers.
//   This routine controls the selection of code optimized
//   for MMX(TM) technology. One iDCT type is supported,
//   using an AAN algorithm.
//
// Context
//
// Returns
//   Valid error code, or 0 if OK.
//
// Parameters
//   *state   IJL state variables.
//   *scan    Current Scan variables.
//   *jprops  IJL variables structure.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EncodeScanBaseline(
  JPEG_PROPERTIES* jprops,
  SCAN*            scan,
  STATE*           state)
{
  int            i, j, jj;
  int            k, l, v, h;
  int            tot_8x8_in_MCU = 0;
  int            blocks_per_MCU[4];
  short*         MCUptr;
  Ipp16u*        q_table[4];
  HUFFMAN_TABLE* dc_table[4];
  HUFFMAN_TABLE* ac_table[4];
  IJL_CONTEXT*   ctx;
  IJLERR         jerr = IJL_OK;

  ctx = (IJL_CONTEXT*)state->ctx;

  for(k = 0; k < scan->ncomps; k++)
  {
    if(NULL == jprops->jscan)
    {
      scan->dc_diff[k] = 0;
    }

    blocks_per_MCU[k] = scan->comps[k].vsampling * scan->comps[k].hsampling;
    tot_8x8_in_MCU += blocks_per_MCU[k];

    dc_table[k] = jprops->jEncFmtDcHuffman[jprops->HuffIdentifierDC[k]];
    ac_table[k] = jprops->jEncFmtAcHuffman[jprops->HuffIdentifierAC[k]];
    q_table[k]  = jprops->jEncFmtQuant[jprops->jframe.comps[k].quant_sel];
  }

  for(j = scan->curyMCU; j < jprops->numyMCUs; j++)
  {
    for(i = scan->curxMCU; i < jprops->numxMCUs; i++)
    {
      /* clear MCU buffer pointer. */
      MCUptr = (short*)jprops->MCUBuf;

      if(!jprops->raw_coefs) /* ss and cc if !raw data only */
      {
        /* Perform color conversion and downsampling on a whole MCU */
        ctx->__g_cc_and_ss_mcu(jprops,i,j);
      }
      else
      {
        /* input blocks with raw data or dct coefficients */
        for(k = 0; k < scan->ncomps; k++)
        {
          for(v = 0; v < scan->comps[k].vsampling; v++)
          {
            for(h = 0; h < scan->comps[k].hsampling; h++)
            {
              /*
              //                    [][]
              // rewrite block from [][] to [][][][] order
              */

              unsigned short* cvPtr = jprops->raw_coefs->raw_ptrs[k] +
                (j * jprops->numxMCUs * scan->comps[k].hsampling *
                                        scan->comps[k].vsampling * DCTSIZE2) +
                (v * DCTSIZE2 * scan->comps[k].hsampling * jprops->numxMCUs) +
                (i * jprops->jframe.comps[k].hsampling * 8) + h * 8;
              for(jj = 0; jj < 8; jj++, cvPtr += (jprops->numxMCUs * scan->comps[k].hsampling * 8), MCUptr += 8)
              {
                IPPCALL(ippsCopy_8u)((const Ipp8u*)cvPtr,(Ipp8u*)MCUptr,sizeof(short) << 3);
              }
            }
          }
        }

        MCUptr = (short*)jprops->MCUBuf;

        if(jprops->raw_coefs->data_type)
        {
          for(k = tot_8x8_in_MCU * DCTSIZE2; k-- ; MCUptr++)
          {
            *MCUptr -= 128;
          }
          MCUptr = (short*)jprops->MCUBuf;
        }
      }

      if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
      {
        /* Perform forward DCTs on a whole MCU */
        for(k = 0; k < tot_8x8_in_MCU; k++)
        {
          fDCT8x8(MCUptr + (k << 6));
        }
      }

      /* Quantize and encode the whole MCU */
      for(k = 0; k < scan->ncomps; k++)
      {
        for(l = 0; l < blocks_per_MCU[k]; l++)
        {

          if(!jprops->raw_coefs || jprops->raw_coefs->data_type)
          {
            fQnt8x8(MCUptr,q_table[k]);
          }

          /* huffman encode */
          jerr = EncodeHuffman8x8(
            MCUptr,
            (Ipp16s*)&scan->dc_diff[k],
            dc_table[k],
            ac_table[k],
            state,
            0);

          if(IJL_OK != jerr)
          {
            goto Exit;
          }

          MCUptr += DCTSIZE2;

        } /* for blocks_per_MCU */
      }   /* for scan->ncomps */

      if(jprops->jframe.restart_interv)
      {
        /* Increment the interval counter (loop back to 0 if counter is */
        /* an even multiple of the restart interval. */
        scan->restart_interv++;

        /* Encode the restart interval. */
        if(!(scan->restart_interv % jprops->jframe.restart_interv))
        {
          /* Flush output buffers. Works even on non-file writing! */
          unsigned int val;
          val = (scan->restart_interv / jprops->jframe.restart_interv - 1) % 8;

          /* Restart the counter. */
          if(val == 7)
          {
            scan->restart_interv = 0;
          }

          jerr = EP_Write_RST(val,state);

          if(IJL_OK != jerr)
          {
            goto Exit;
          }

          /* Reset the entropy statistics */
          for(k = 0; k < scan->ncomps; k++)
          {
            scan->dc_diff[k] = 0;
          }
        }
      }

      /* process interrupt */
      if(jprops->interrupt != 0)
      {
        jprops->roi.right  = i;
        jprops->roi.bottom = j;

        scan->curxMCU = (i + 1) % jprops->numxMCUs;

        if(scan->curxMCU == 0)
        {
          scan->curyMCU = j + 1;
        }
        else
        {
          scan->curyMCU = j;
        }

        jerr = IJL_INTERRUPT_OK;
        goto Exit;
      }

    } /* for numxMCUs */
  }   /* for numyMCUs */

  /* flusf huffman state */
  jerr = EncodeHuffman8x8(0,0,dc_table[0],ac_table[0],state,1);

  if(IJL_OK != jerr)
  {
    goto Exit;
  }

Exit:

  return jerr;
} /* EncodeScanBaseline() */


/*F*
////////////////////////////////////////////////////////////////////////////
//  Name
//    EncodeScanProgressive
//
//  Purpose
//    Encode a scan of data.
//    Currently the scan does not include restart markers.
//
//  Context
//
//  Returns
//    Valid error code, or 0 if OK.
//
//  Parameters
//    jprops  IJL variables structure.
//    pScan    Current Scan variables.
//    pState   IJL state variables.
//
////////////////////////////////////////////////////////////////////////////
*F*/

OWNFUN(IJLERR) EncodeScanProgressive(
  JPEG_PROPERTIES* jprops,
  SCAN*            pScan,
  STATE*           pState)
{
  int     i, j;
  int     k, l;
  int     n, m;
  int     idx;
  int     pos;
  int     comp_id;
  int     offset;
  int     mcu_size;
  int     mcu_row_size;
  int     blocks_per_mcu;
  int     blocks_per_component[4];
  Ipp16s* coef_buf;
  IppStatus status;
  IJLERR jerr = IJL_OK;


  TRACE0(trCALL|trBEFORE,"enter in Encode_Scan_Progressive()\n");

  for(k = 0, blocks_per_mcu = 0; k < jprops->jframe.ncomps; k++)
  {
    blocks_per_component[k] = jprops->jframe.comps[k].vsampling *
                              jprops->jframe.comps[k].hsampling;

    blocks_per_mcu += blocks_per_component[k];
  }

  mcu_size     = blocks_per_mcu * DCTSIZE2;
  mcu_row_size = mcu_size * jprops->numxMCUs;

  for(k = 0; k < pScan->ncomps; k++)
  {
    idx = pScan->comps[k].comp;
    pScan->dc_diff[k] = 0;
    pScan->comps[k].dc_table = jprops->jEncFmtDcHuffman[jprops->HuffIdentifierDC[idx]];
    pScan->comps[k].ac_table = jprops->jEncFmtAcHuffman[jprops->HuffIdentifierAC[idx]];
  }

  if(pScan->start_spec == 0 && pScan->end_spec == 0)
  {
    /* encode DC scan */
    for(j = 0; j < jprops->numyMCUs; j++)
    {
      for(i = 0; i < jprops->numxMCUs; i++)
      {
        coef_buf = jprops->coef_buffer + j * mcu_row_size + i * mcu_size;

        for(k = 0; k < pScan->ncomps; k++)
        {
          for(l = 0; l < blocks_per_component[k]; l++)
          {
            /* huffman encode */
            Encode_Prog_DC_SA(
              coef_buf,
              (Ipp16s*)&pScan->dc_diff[k],
              pScan->approx_high,
              pScan->approx_low,
              pScan->comps[k].dc_table,
              pState,
              0);
            coef_buf += DCTSIZE2;
          } /* for block in components */
        } /* for components */
      } /* for numxMCUs */
    } /* for numyMCUs */

    /* protect against buffer overrun */
    if(pState->entropy_bytes_left <= 128)
    {
      if(NULL == pState->file)
      {
        jerr = IJL_BUFFER_TOO_SMALL;
        goto Exit;
      }

      jerr = Flush_Buffer_To_File(pState);
      if(IJL_OK != jerr)
        goto Exit;
    }

    pos = 0;

    /* flush huffman state */
    if(pScan->approx_high == 0)
    {
      status = IPPCALL(ippiEncodeHuffman8x8_DCFirst_JPEG_16s1u_C1)(
        0,
        pState->cur_entropy_ptr,
        pState->entropy_bytes_left,
        &pos,
        0,
        pScan->approx_low,
        0,
        pState->u.pEncHuffState,
        1);
    }
    else
    {
      status = IPPCALL(ippiEncodeHuffman8x8_DCRefine_JPEG_16s1u_C1)(
        0,
        pState->cur_entropy_ptr,
        pState->entropy_bytes_left,
        &pos,
        pScan->approx_low,
        pState->u.pEncHuffState,
        1);
    }

    pState->cur_entropy_ptr         += pos;
    pState->entropy_bytes_left      -= pos;
    pState->entropy_bytes_processed += pos;

    if(ippStsNoErr != status)
    {
      jerr = IJL_INTERNAL_ERROR;
      goto Exit;
    }
  }
  else
  {
    comp_id = pScan->comps->comp;

    for(i = 0, offset = 0; i < comp_id; i++)
    {
      offset += blocks_per_component[i];
    }

    /* encode AC scan */
    for(j = 0; j < jprops->numyMCUs; j++)
    {
      for(m = 0; m < pScan->comps[0].vsampling; m++)
      {
        /* Skip the last block row of the image if greater than the image height. */
        if(((j*pScan->comps[0].vsampling*8) + (m*8)) >= jprops->JPGHeight)
        {
          break;
        }

        for(i = 0; i < jprops->numxMCUs; i++)
        {
          coef_buf = jprops->coef_buffer + j * mcu_row_size + i * mcu_size;
          coef_buf += (offset * DCTSIZE2 + m * pScan->comps->vsampling * DCTSIZE2);

          for(n = 0; n < pScan->comps->hsampling; n++)
          {
            /* Ignore the last column(s) of the image. */
            if(((i*pScan->comps[0].hsampling*8) + (n*8)) >= jprops->JPGWidth)
            {
              break;
            }

            /* huffman encode */
            if(pScan->approx_high == 0)
            {
              Encode_Prog_AC_first(
                coef_buf,
                pScan->start_spec,
                pScan->end_spec,
                pScan->approx_low,
                pScan->comps[0].ac_table,
                pState,
                0);
            }
            else
            {
              Encode_Prog_AC_refine(
                coef_buf,
                pScan->start_spec,
                pScan->end_spec,
                pScan->approx_low,
                pScan->comps[0].ac_table,
                pState,
                0);
            } /* pScan->approx_high */
            coef_buf += DCTSIZE2;
          } /* pScan->comps->hsampling */
        } /* for numxMCUs */
      } /* pScan->comps[0].vsampling */
    } /* for numyMCUs */

    /* protect against buffer overrun */
    if(pState->entropy_bytes_left <= 128)
    {
      if(NULL == pState->file)
      {
        jerr = IJL_BUFFER_TOO_SMALL;
        goto Exit;
      }

      jerr = Flush_Buffer_To_File(pState);
      if(IJL_OK != jerr)
        goto Exit;
    }

    pos = 0;

    /* flush huffman state */
    if(pScan->approx_high == 0)
    {
      status = IPPCALL(ippiEncodeHuffman8x8_ACFirst_JPEG_16s1u_C1)(
        0,
        pState->cur_entropy_ptr,
        pState->entropy_bytes_left,
        &pos,
        pScan->start_spec,
        pScan->end_spec,
        pScan->approx_low,
        pScan->comps[0].ac_table->u.pEncHuffTbl,
        pState->u.pEncHuffState,
        1);
    }
    else
    {
      status = IPPCALL(ippiEncodeHuffman8x8_ACRefine_JPEG_16s1u_C1)(
        0,
        pState->cur_entropy_ptr,
        pState->entropy_bytes_left,
        &pos,
        pScan->start_spec,
        pScan->end_spec,
        pScan->approx_low,
        pScan->comps[0].ac_table->u.pEncHuffTbl,
        pState->u.pEncHuffState,
        1);
    }

    pState->cur_entropy_ptr         += pos;
    pState->entropy_bytes_left      -= pos;
    pState->entropy_bytes_processed += pos;
  } /* Ss && Se */

Exit:

  TRACE0(trCALL|trAFTER,"leave Encode_Scan_Progressive()\n");

  return jerr;
} /* EncodeScanProgressive() */


