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
#ifndef __INIT_H__
#include "init.h"
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




OWNFUN(BOOL) ownInitializeLibrary(
  IJL_CONTEXT* ctx)
{
  //ippStaticInit();

  ctx->__g_cc_and_ss_mcu = NULL;
  ctx->__g_us_and_cc_mcu = NULL;

  return 1;
} /* ownInitializeLibrary() */


OWNFUN(BOOL) ownFreeLibrary(
  IJL_CONTEXT* ctx)
{
  ctx->__g_cc_and_ss_mcu = NULL;
  ctx->__g_us_and_cc_mcu = NULL;

  return 1;
} /* ownFreeLibrary() */

