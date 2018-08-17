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
#ifndef __FILEIO_H__
#include "interface_manager/fileio.h"
#endif



#ifdef __INTEL_COMPILER
#pragma warning(disable:171)
#endif


OWNFUN(IJL_HANDLE) ownOpenFile(
  const char*   szFileName,
  IJL_OPEN_MODE mode)
{
  IJL_HANDLE hFile = INVALID_HANDLE_VALUE;

  TRACE0(trCALL|trBEFORE,"enter in ownOpenFile\n");

#if defined (_WIN32)
  {
    unsigned int dwCreationDistribution;

    if(mode & GENERIC_WRITE)
    {
      /* OPEN_ALWAYS - don't truncate file */
      dwCreationDistribution = CREATE_ALWAYS;
    }
    else
    {
      dwCreationDistribution = OPEN_EXISTING;
    }

#if !defined (_WINCE)
    hFile = CreateFileA(szFileName,mode,FILE_SHARE_READ,NULL,dwCreationDistribution,0,NULL);
#else
    WCHAR wchBuff[MAX_PATH];
    __g_memzero(&wchBuff[0],sizeof(wchBuff));
    MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,szFileName,strlen(szFileName),&wchBuff[0],sizeof(wchBuff));
    hFile = CreateFileW(&wchBuff[0],mode,FILE_SHARE_READ,NULL,dwCreationDistribution,0,NULL);
#endif

    if(INVALID_HANDLE_VALUE == hFile)
    {
      hFile = NULL;
    }
  }

#else

  {
    if(mode & GENERIC_WRITE)
    {
      /* OPEN_ALWAYS - don't truncate file */
      hFile = fopen(szFileName,"wb");
    }
    else
    {
      hFile = fopen(szFileName,"rb");
    }
  }

#endif

  TRACE2(trINFO,"file %s was open with handle 0x%08X\n",szFileName,hFile);
  TRACE0(trCALL|trAFTER,"leave ownOpenFile\n");

  return hFile;
} /* ownOpenFile() */


OWNFUN(BOOL) ownCloseFile(
  IJL_HANDLE hFile)
{
  BOOL bres = TRUE;
  TRACE0(trCALL|trBEFORE,"enter in ownCloseFile\n");
#if defined (_WIN32)
  bres = CloseHandle(hFile);
#else
  int ires =  fclose((FILE*)hFile);
  if(ires != 0)
  {
    bres = FALSE;
  }
#endif
  TRACE1(trINFO,"file 0x%08X closed\n",hFile);
  TRACE0(trCALL|trAFTER,"leave ownCloseFile\n");
  return bres;
} /* ownCloseFile() */


OWNFUN(BOOL) ownReadFile(
  IJL_HANDLE    hFile,
  void*         buffer,
  unsigned int  size,
  unsigned int* count)
{
  BOOL bres = TRUE;

#if defined (_WIN32)
  bres = ReadFile(hFile,buffer,size,(unsigned long*)count,NULL);
#else
  *count = fread(buffer,sizeof(Ipp8u),size,(FILE*)hFile);
  if(*count != size)
  {
    bres = FALSE;
  }
#endif

  return bres;
} /* ownReadFile() */


OWNFUN(BOOL) ownWriteFile(
  IJL_HANDLE    hFile,
  void*         buffer,
  unsigned int  size,
  unsigned int* count)
{
  BOOL bres = TRUE;

#if defined (_WIN32)
  bres = WriteFile(hFile,buffer,size,(unsigned long*)count,NULL);
#else
  *count = fwrite(buffer,sizeof(Ipp8u),size,(FILE*)hFile);
  if(*count != size)
  {
    bres = FALSE;
  }
#endif

  return bres;
} /* ownWriteFile() */


OWNFUN(BOOL) ownSeekFile(
  IJL_HANDLE   hFile,
  unsigned int offset,
  unsigned int mode)
{
  BOOL  bres = TRUE;
  unsigned int res;
#if defined (_WIN32)
  res = SetFilePointer(hFile,offset,NULL,mode);
  if(res == 0xffffffff)
  {
    bres = FALSE;
  }
#else
  res = fseek((FILE*)hFile,offset,mode);
  if(0 != res)
  {
    bres = FALSE;
  }
#endif
  return bres;
} /* ownSeekFile() */

